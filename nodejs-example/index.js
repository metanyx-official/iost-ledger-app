const Iost = require("./lib/iost");
const Ledger = require("./lib/ledger");
const Utils = require("./lib/utils");
const Config = require("config");
const Assert = require("assert");

// if (process.env.NODE_ENV === "development") {
//     require("axios-debug-log")({
//         request: (debug, config) => {
//           debug('Request with ' + config.headers["content-type"]);
//           debug('Request config = ' + JSON.stringify(config));
//           console.log("----------------Request----------------");
//           console.log(config);
//         },
//         response: (debug, response) => {
//             debug(
//                 'Response with ' + response.headers["content-type"],
//                 'from ' + response.config.url
//             );
//             console.log("----------------Response----------------");
//             console.log(config);
//         },
//         error: (debug, error) => {
//             // Read https://www.npmjs.com/package/axios#handling-errors for more info
//             debug('Boom', error)
//             console.log("---------------Boom-----------------");
//             console.log(error);
//         }
//       });
// }

const iost = new Iost(Config.iost.api.url, Config.iost.api.chain);
const ledger = new Ledger("IOST", Config.ledger.transport);
const BIP32_PATH_BASE = "44'/291'";

Utils.delay().then(ledger.open).then(() => {
  return ledger.getConfiguration(BIP32_PATH_BASE);
}).then((ledgerConfig) => {
  const kp = iost.newKeyPair();
  const admin = iost.login(Config.iost.account.name, Config.iost.account.key);
  const transaction = iost.newAccount(
    "a",
    admin.getID(),
    kp.id
  );

  Assert.deepStrictEqual(ledgerConfig, {hasStorage: false, version: "1.0.2"});
  Assert.strictEqual(transaction.gasRatio, 1);
  Assert.strictEqual(transaction.gasLimit, 1000000);
  Assert.deepEqual(transaction.signers, []);
  Assert.deepEqual(transaction.signatures, []);
  Assert.deepStrictEqual(transaction.amount_limit, [iost.amountOf(10)]);
  Assert.strictEqual(transaction.chain_id, 1024);
  Assert.strictEqual(transaction.reserved, null);
  Assert.strictEqual(transaction.delay, 0);

  return iost.broadcast(transaction);
}).then((trx) => {
  return Utils.callAsync(
    ledger.getPublicKey.bind(ledger, BIP32_PATH_BASE + "/0'", 1, 1),
    [trx]
  );
}).then((asyncCallResult) => {
  console.log("pubKey:", JSON.stringify(asyncCallResult));

  const transaction = iost.newAccount(
    "b",
    iost.getTransactionSignUpReceipt(asyncCallResult[1])[0],
    asyncCallResult[0].base58
  );
  console.log("----");
  console.log("trx:", transaction);
  console.log("----");
  Assert.strictEqual(transaction.gasRatio, 1);
  Assert.strictEqual(transaction.gasLimit, 1000000);
  Assert.deepEqual(transaction.signers, []);
  Assert.deepEqual(transaction.signatures, []);
  Assert.deepStrictEqual(transaction.amount_limit, [iost.amountOf(10)]);
  Assert.strictEqual(transaction.chain_id, 1024);
  Assert.strictEqual(transaction.reserved, null);
  Assert.strictEqual(transaction.delay, 0);


  return Utils.callAsync(
    ledger.signTransaction.bind(
        ledger, BIP32_PATH_BASE + "/0'",
        iost.getTransactionBytes(transaction)
    ),
    [transaction]
  );
}).then((asyncCallResult) => {
    console.log("signTransaction:", JSON.stringify(asyncCallResult));

//  transaction.addPublishSign(this._id, this._key_pair["active"])
//  this.publisher = publisher;
//  const info = this._publish_hash();
//  const sig = new Signature(info, kp);
//  this.publisher_sigs.push(sig)

//  asyncCallResult[1].addPublishSign(
//    iost.getTransactionSignUpReceipt(asyncCallResult[1]),
//    iost.newKeyPair()
//  );
  asyncCallResult[1].publisher = iost.getTransactionSignUpReceipt(asyncCallResult[1]);
  asyncCallResult[1].publisher_sigs.push(iost.makeSignature(asyncCallResult[0]));
  console.log("broadcast:", JSON.stringify(transaction));
  return iost.broadcast(transaction);
}).then((response) => {
  console.log(response);
  console.log(JSON.stringify(response));
  console.log("DONE");
  return ledger.close();
}).catch(Utils.fail);


// delay().then(() => {
//     const kp = IOST.KeyPair.newKeyPair();
//     const tx = iost.newAccount(
//             myid,
//             "admin",
//             kp.id,
//             kp.id,
//             1024,
//             1000
//         );
//     account.signTx(tx);
//     console.log("Tx: ", tx);
//     console.dir(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onFailed((response) => {
// 	     console.error('Tx failed:', response);
//              console.dir(response);
// 	})
//         .onSuccess((response) => {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             accountList[0] = new IOST.Account(myid);
//             accountList[0].addKeyPair(kp, "owner");
//             accountList[0].addKeyPair(kp, "active");
//         })
//         .send()
//         .listen(1000, 5);
//     console.log('IOST.TxHandler-70');
//     return checkHandler(handler);
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "addPermission", [myid, "perm1", 1]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             console.log(JSON.stringify(accountInfo), typeof(accountInfo));
//             Assert.notEqual(JSON.stringify(accountInfo).indexOf(`"perm1":{"name":"perm1","group_names":[],"items":[],"threshold":"1"}}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-86');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "dropPermission", [myid, "perm1"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.equal(JSON.stringify(accountInfo).indexOf(`"perm1":{"name":"perm1","group_names":[],"items":[],"threshold":"1"}}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-102');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "assignPermission", [myid, "active", "IOST1234", 1]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.notEqual(JSON.stringify(accountInfo).indexOf(`{"id":"IOST1234","is_key_pair":true,"weight":"1","permission":""}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-117');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "revokePermission", [myid, "active", "IOST1234"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.equal(JSON.stringify(accountInfo).indexOf(`{"id":"IOST1234","is_key_pair":true,"weight":"1","permission":""}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-132');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "addGroup", [myid, "grp0"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.notEqual(JSON.stringify(accountInfo).indexOf(`"groups":{"grp0":{"name":"grp0","items":[]}}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-147');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "assignGroup", [myid, "grp0", "acc1@active", 1]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.notEqual(JSON.stringify(accountInfo).indexOf(`{"grp0":{"name":"grp0","items":[{"id":"acc1","is_key_pair":false,"weight":"1","permission":"active"}]}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-162');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "revokeGroup", [myid, "grp0", "acc1@active"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.equal(JSON.stringify(accountInfo).indexOf(`{"grp0":{"name":"grp0","items":[{"id":"acc1","is_key_pair":false,"weight":"1","permission":"active"}]}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     console.log('IOST.TxHandler-177');
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "assignPermissionToGroup", [myid, "active", "grp0"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.notEqual(JSON.stringify(accountInfo).indexOf(`"group_names":["grp0"]`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "revokePermissionInGroup", [myid, "active", "grp0"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.equal(JSON.stringify(accountInfo).indexOf(`"group_names":["grp0"]`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     return checkHandler(handler)
// })
// .then(function () {
//     const tx = iost.callABI("auth.iost", "dropGroup", [myid, "grp0"]);
//     accountList[0].signTx(tx);
//     const handler = new IOST.TxHandler(tx, rpc);
//     handler
//         .onSuccess(async function (response) {
//             console.log("Success... tx, receipt: "+ JSON.stringify(response));
//             let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
//             Assert.equal(JSON.stringify(accountInfo).indexOf(`"groups":{"grp0":{"name":"grp0","items":[]}}`), -1)
//         })
//         .send()
//         .listen(1000, 10);
//     return checkHandler(handler)
// })
// .catch(Failed);

// send a call
//let handler = iost.callABI("iost.token", "transfer", ["iost", "form", "to", "1000.000"]);

//handler
//    .onPending(console.log)
//    .onSuccess(console.log)
//    .onFailed(console.log)
//    .send()
//    .listen(); // if not listen, only onPending or onFailed (at sending tx) will be called
