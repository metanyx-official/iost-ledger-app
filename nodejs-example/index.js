const Iost = require("./lib/iost");
const Ledger = require("./lib/ledger");
const Utils = require("./lib/utils");
const Config = require("config");
const Assert = require("assert");

const iost = new Iost(Config.iost.api.url, Config.iost.api.chain);
const ledger = new Ledger("IOST", Config.ledger.transport);
const BIP32_PATH_BASE = "44'/291'";

const createTrxAction = (action, actionName, contract) => {
  contract = contract || iost.TRX_CONTRACT_AUTH;
  return {
    contract,
    actionName,
    data: JSON.stringify(action).replace(' ', '')
  };
};
const createTrxReceipt = (receipt, funcName) => {
  return {
    func_name: funcName,
    data: JSON.stringify(receipt).replace(' ', '')
  };
};
const createTrxActionSignup = (action) => createTrxAction(action, iost.TRX_METHOD_SIGNUP);
const createTrxReceiptSignup = (receipt) => createTrxReceipt(
  receipt,
  iost.TRX_CONTRACT_AUTH + "/" + iost.TRX_METHOD_SIGNUP
);

const checkTrx = (trx, chainId) => {
  Assert.strictEqual(trx.gasRatio, 1);
  Assert.strictEqual(trx.gasLimit, 1000000);
  Assert.deepEqual(trx.signers, []);
  Assert.deepEqual(trx.signatures, []);
  Assert.strictEqual(trx.chain_id, chainId || 1024);
  Assert.strictEqual(trx.reserved, null);
  Assert.strictEqual(trx.delay, 0);
  Assert.deepStrictEqual(trx.amount_limit, [iost.amountOf(20)]);
};
const checkCommit = (trx, commit) => {
  checkTrx(trx, 1023);
  Assert.strictEqual(commit.status_code, iost.STATUS_CODE_SUCCESS);
  Assert.strictEqual(commit.message, "");
  Assert.deepStrictEqual(commit.returns, ['[""]', '[]']);
};
const checkTransfer = (commit) => {
  Assert.strictEqual(commit.status_code, iost.STATUS_CODE_SUCCESS);
  Assert.strictEqual(commit.message, "");
  Assert.deepStrictEqual(commit.returns, ['[]']);
};


Utils.delay().then(ledger.open).then(async () => {
  const ledgerConfig = await ledger.getConfiguration(BIP32_PATH_BASE);
  Assert.deepStrictEqual(ledgerConfig, {hasStorage: false, version: "0.0.1"});

  let account = iost.login(Config.iost.account.name, Config.iost.account.key);
  const keyPairA = iost.createKeyPair();
  const pubKeyA = keyPairA.id;
  const trxA = iost.newAccount("a", account.getID(), pubKeyA, 1, 10);
  const trxInfoA = iost.getTxInfo(trxA).findSignUpInActions();

  checkTrx(trxA);
  Assert.strictEqual(trxInfoA[1], pubKeyA);
  Assert.strictEqual(trxInfoA[2], pubKeyA);
  Assert.deepStrictEqual(trxA.actions[0], createTrxActionSignup(trxInfoA));

  const commitA = await iost.commitTx(trxA, account);
  const pubKeyB = await ledger.getPublicKey(BIP32_PATH_BASE + "/0'", 1, 1);
  const trxB = iost.newAccount("b", trxInfoA[0], pubKeyB.base58, 1, 10);
  const trxInfoB = iost.getTxInfo(trxB).findSignUpInActions();

  checkCommit(trxA, commitA);
  Assert.deepStrictEqual(iost.getTxInfo(commitA).findSignUpInReceipts(), trxInfoA);
  checkTrx(trxB);
  Assert.deepStrictEqual(trxB.actions[0], createTrxActionSignup(trxInfoB));
  Assert.strictEqual(trxInfoB[1], pubKeyB.base58);
  Assert.strictEqual(trxInfoB[2], pubKeyB.base58);

  checkTransfer(await iost.commitTx(iost.newTranfer(account.getID(), trxInfoA[0], 88), account));
  account = iost.makeAccount(trxInfoA[0], keyPairA);

  const commitB = await iost.commitTx(trxB, account);
  const pubKeyC = await ledger.getPublicKey(BIP32_PATH_BASE + "/1'", 1, 1);
  const trxC = iost.newAccount("c", trxInfoB[0], pubKeyC.base58, 1, 10);
  const trxInfoC = iost.getTxInfo(trxC).findSignUpInActions();

  checkCommit(trxB, commitB);
  Assert.deepStrictEqual(iost.getTxInfo(commitB).findSignUpInReceipts(), trxInfoB);
  checkTrx(trxC);
  Assert.deepStrictEqual(trxC.actions[0], createTrxActionSignup(trxInfoC));
  Assert.strictEqual(trxInfoC[1], pubKeyC.base58);
  Assert.strictEqual(trxInfoC[2], pubKeyC.base58);

  checkTransfer(await iost.commitTx(iost.newTranfer(account.getID(), trxInfoB[0], 44), account));
  account = {
    name: trxInfoB[0],
    pubKey: iost.base58ToArray(pubKeyB.base58),
    sign: async (bytes) => {
      return await ledger.signTransaction(BIP32_PATH_BASE + "/0'", bytes)
    }
  };

  const commitC = await iost.commitTx(trxC, account);
  checkCommit(trxC, commitC);
  Assert.deepStrictEqual(iost.getTxInfo(commitC).findSignUpInReceipts(), trxInfoC);

  // Ready to send money
  checkTransfer(await iost.commitTx(iost.newTranfer(account.name, trxInfoC[0], 1), account));
  console.log(account.name, pubKey, iost.getBalance(account.name));

}).then(ledger.close).catch(Utils.fail);

//const privKeyHex = "7102CD6851DED18677F3CF323011B468881F2B2F31F77769FBAD9060A86A7CFD";
//const privKeyBytes = Utils.hexStringToArray(privKeyHex);
//console.log("1---", privKeyBytes);
//const kp = iost.makeKeyPair(privKeyBytes);
//console.log("2---", kp.B58PubKey());
//console.log("3---", kp.B58SecKey());


////pubKeyC: HA5XnNtfAMS1xwaEcQuyqsH7v6EboAzhaNoqoYRpSG2H
//    let admin = iost.login(Config.iost.account.name, Config.iost.account.key);
//    const userName = "u50730957b";
//    const userKey = "EV4KqtPf99H6xjrib9tyL4EVwyVa7MgVZVXGimwhZFMU";
//    const pubKeyB = await ledger.getPublicKey(BIP32_PATH_BASE + "/0'", 1, 1);
//    const pubKeyC = await ledger.getPublicKey(BIP32_PATH_BASE + "/1'", 1, 1);
//    const trxC = iost.newAccount("c", userName, pubKeyC.base58, 1, 10);
//    const trxInfoC = iost.getTxInfo(trxC).findSignUpInActions();

//    checkTrx(trxC);
//    Assert.deepStrictEqual(trxC.actions[0], createTrxActionSignup(trxInfoC));
//    Assert.strictEqual(trxInfoC[1], pubKeyC.base58);
//    Assert.strictEqual(trxInfoC[2], pubKeyC.base58);

////    console.log("--C--");
////    console.log("trx:", trxC);
////    console.log("trxInfo:", trxInfoC);
////    console.log("pubKey:", pubKeyC);
////    console.log("--C--");

//    account = {
//        name: userName,
//        pubKey: iost.base58ToArray(pubKeyB.base58),
//        sign: async (bytes) => {
//            console.log("Signing", bytes);
//            return await ledger.signTransaction(BIP32_PATH_BASE + "/0'", bytes)
//        }
//    };
