const Iost = require('iost');
const Utils = require('./utils');
const NaCl = require('tweetnacl');

const hostListChain1023 = [
  "//api.iost.io",
  "//127.0.0.1",
  "//localhost"
];

module.exports = function(apiUrl, chainId) {
  const self = this;
  const iost = new Iost.IOST({
    gasRatio: 1,
    gasLimit: 1000000,
    delay: 0,
    expiration: 90,
    defaultLimit: "1000"
  });

  iost.setRPC(new Iost.RPC(new Iost.HTTPProvider(apiUrl, 55555)));
  iost.setAccount(new Iost.Account("empty"));

  chainId = chainId || (
    (apiUrl.indexOf(hostListChain1023[1]) < 0) &&
    (apiUrl.indexOf(hostListChain1023[2]) < 0) &&
    (apiUrl.indexOf(hostListChain1023[3]) < 0)
  ) ? 1023 : 1024;

  this.TOKEN_NAME = "iost";
  this.TOKEN_TAG = "token." + this.TOKEN_NAME;
  this.TRX_CONTRACT_AUTH = "auth.iost";
  this.TRX_METHOD_SIGNUP = "signUp";
  this.STATUS_CODE_SUCCESS = "SUCCESS";

  this.arrayToBase58 = (array) => Iost.Bs58.encode(array);
  this.base58ToArray = (str) => Iost.Bs58.decode(str);
  this.createSignature = (json) => Iost.Signature.fromJSON(json);
  this.makeSignature = (bytes, keyPair) => new Iost.Signature(bytes, keyPair);
  this.createKeyPair = () => Iost.KeyPair.newKeyPair();
  this.makeKeyPair = (secret) => new Iost.KeyPair(secret);
  this.amountOf = (value, token) => {
    token = token || self.TOKEN_NAME;
    value = value || 0;
    value = value.toString();
    return {token, value};
  };
  this.getBalance = async (userName, token) => {
    return await iost.currentRPC.blockchain.getBalance(userName, token || self.TOKEN_NAME);
  };
  this.login = (userName, key) => {
    const keyPair = new Iost.KeyPair(Iost.Bs58.decode(key));
    const account = self.makeAccount(userName, keyPair);
    iost.setAccount(account);
      return account;
    };
    this.makeAccount = (userName, keyPair) => {
      const account = new Iost.Account(userName);
      if (keyPair) {
        account.addKeyPair(keyPair, "owner");
        account.addKeyPair(keyPair, "active");
      }
      return account;
    };
    this.newAccount = (nameSuffix, creator, pubKey, initialRAM, initialGasPledge) => {
      const prefix = Date.now().toString();
      const trx = iost.newAccount(
        "u" + prefix.substr(prefix.length - 8) + nameSuffix,
        creator,
        pubKey,
        pubKey,
        initialRAM || 0,
        initialGasPledge || 0
      );
      trx.addApprove(self.TOKEN_NAME, +10 + (initialGasPledge || 0));
      return trx;
    };
    this.newTranfer = (from, to, amount, memo, token) => {
      token = token || self.TOKEN_NAME;
      memo = memo || "";
      amount = amount.toString();

      const trx = iost.callABI(self.TOKEN_TAG, "transfer", [token, from, to, amount, memo]);
      trx.addApprove(token, amount);
      return trx;
    };
    this.getTxInfo = (trx) => {
      const filterBy = (array, key, value) => array.filter(x => x[key] === value);
      const findAction = (action) => filterBy(trx.actions, "actionName", action);
      const findReceipt = (funcName) => filterBy(trx.receipts, "func_name", funcName);
      return {
        findSignUpInActions: () => JSON.parse(findAction(self.TRX_METHOD_SIGNUP)[0].data),
        findSignUpInReceipts: () => JSON.parse(findReceipt(self.TRX_CONTRACT_AUTH + "/" + self.TRX_METHOD_SIGNUP)[0].content)
      };
    };
    this.commitTx = async (trx, publisher) => {
      trx.setTime(iost.config.expiration, iost.config.delay, iost.serverTimeDiff);

      if (trx.chain_id !== chainId) {
        trx.setChainID(chainId);
      }
      if (publisher === undefined) {
        iost.currentAccount.signTx(trx);
      } else if (publisher instanceof Iost.Account) {
        publisher.signTx(trx);
      } else {
        const algorithm = Iost.Algorithm.Ed25519;
        const pubkey = publisher.pubKey;
        const sig = await publisher.sign(trx._publish_hash());
        const signature = Object.assign(
          new Iost.Signature,
          {algorithm, pubkey, sig}
        );
        trx.publisher = publisher.name;
        trx.publisher_sigs.push(signature);
      }

      let answer = await iost.currentRPC.transaction.sendTx(trx);
      let hash = answer.hash;

      for (let i = 0; i < iost.config.expiration; i++) {
        if (answer.status_code === self.STATUS_CODE_SUCCESS) {
          if (i > 0) {
            return answer;
          }
        } else if (answer.status_code) {
          throw Error(answer.message);
        }
        await Utils.delay(i * iost.config.expiration);
        try {
          answer = await iost.currentRPC.transaction.getTxReceiptByTxHash(hash);
        } catch (error) {
          if (error instanceof Error &&
            error.message !== "error: {\"code\":2,\"message\":\"failed to Get the receipt: not found\"}"
          ) {
            throw error;
          }
        }
      }
      throw Error("Receipt not found for trx hash " + hash);
    };
};
