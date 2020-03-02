const Iost = require('iost');
const Utils = require('./utils');

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
    chainId = chainId || (
        (apiUrl.indexOf(hostListChain1023[1]) < 0) &&
        (apiUrl.indexOf(hostListChain1023[2]) < 0) &&
        (apiUrl.indexOf(hostListChain1023[3]) < 0)
    ) ? 1023 : 1024;

    // let tokenSym = Date.now().toString();
    // tokenSym = "t" + tokenSym.substr(tokenSym.length - 4);
    iost.setRPC(new Iost.RPC(new Iost.HTTPProvider(apiUrl, 4444)));
    iost.setAccount(new Iost.Account("empty"));
    
    this.makeSignature = (json) => Iost.Signature.fromJSON(json);
    this.newKeyPair = () => Iost.KeyPair.newKeyPair();
    this.amountOf = (value, token) => {
        token = token || "iost";
        value = value || 0;
        value = value.toString();
        return {token, value};
    };
    this.getAccount = (name, kp) => {
        const result = new Iost.Account(name);
        if (kp) {
            result.addKeyPair(kp, "owner");
            result.addKeyPair(kp, "active");
        }
        return result;
    };
    this.newAccount = (name, creator, pubKey, initialRAM, initialGasPledge) => {
        const prefix = Date.now().toString();

        initialRAM = initialRAM || 0;
        initialGasPledge = initialGasPledge || 0;
        result = iost.newAccount(
            "u" + prefix.substr(prefix.length - 8) + name,
            creator,
            pubKey,
            pubKey,
            initialRAM,
            initialGasPledge
        );
        result.addApprove("iost", +10);

        return result;
        // parent.signTx(tx);
        // console.log("Tx: ", tx);
        // console.dir(tx);

        // const handler = new IOST.TxHandler(tx, rpc);
        // handler
        //     .onFailed((response) => {
        //     console.error('Tx failed:', response);
        //         console.dir(response);
        // })
        //     .onSuccess((response) => {
        //         console.log("Success... tx, receipt: "+ JSON.stringify(response));
        //         accountList[0] = new IOST.Account(myid);
        //         accountList[0].addKeyPair(kp, "owner");
        //         accountList[0].addKeyPair(kp, "active");
        //     })
        //     .send()
        //     .listen(1000, 5);
        // console.log('IOST.TxHandler-70');
        // return checkHandler(handler);
    };
    this.login = (name, key) => {
        const kp = new Iost.KeyPair(Iost.Bs58.decode(key));
        const result = self.getAccount(name, kp);
        iost.setAccount(result);
        return result;
    };
    this.tranfer = (from, to, amount, asset) => {
        const token = asset || "iost";
        const result = iost.callABI("iost.token", "transfer", [token, from, to, amount]);
        result.addApprove(token, amount);
        return result;

        // accountList[0].signTx(tx);
        // const handler = new IOST.TxHandler(tx, rpc);
        // handler
        //     .onSuccess(async function (response) {
        //         console.log("Success... tx, receipt: "+ JSON.stringify(response));
        //         let accountInfo = await rpc.blockchain.getAccountInfo(myid, false);
        //         Assert.notEqual(JSON.stringify(accountInfo).indexOf(`{"id":"IOST1234","is_key_pair":true,"weight":"1","permission":""}`), -1)
        //     })
        //     .send()
        //     .listen(1000, 10);
        // console.log('IOST.TxHandler-117');
        // return checkHandler(handler);
    };
    this.getTransactionSignUpReceipt = (trx) => JSON.parse(
        trx.receipts.filter(x => x.func_name === "auth.iost/signUp")[0].content
    );
    this.getTransactionBytes = (trx) => trx._bytes(1);

    this.broadcast = async (transaction) => {
        // const handler = new Iost.TxHandler(transaction, iost.currentRPC);
        // const context = {
        //     data: undefined,
        //     error: undefined
        // };

        transaction.setTime(iost.config.expiration, iost.config.delay, iost.serverTimeDiff);

        if (transaction.chain_id !== chainId) {
            transaction.setChainID(chainId);
        }

        if (transaction.publisher_sigs.length === 0) {
            iost.currentAccount.signTx(transaction);
        }
        // handler
        // .onSuccess(r => context.data = r)
        // .onFailed(e => context.error = e)
        // .send();
        let answer = await iost.currentRPC.transaction.sendTx(transaction);
        let hash = answer.hash;
        for (let i = 0; i < iost.config.expiration; i++) {
            if (answer.status_code === "SUCCESS") {
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
        throw Error("Receipt not found for transacion " + hash);
    };
};

        // return new Promise((resolve, reject) => {
        //     let i = 0;
        //     let id = setInterval(() => {
        //         console.log('listening.......................');
        //         iost
        //         .currentRPC
        //         .transaction
        //         .getTxReceiptByTxHash(hash)
        //         .then((res) => {
        //             if (res.status_code === "SUCCESS") {
        //                 resolve(res);
        //             } else if (res.status_code !== undefined) {
        //                 reject(res);
        //             }
        //         }).catch(reject);
        //         // } else if (handler.status === "success" || handler.status === "failed" || i > 10) {
        //         //     clearInterval(id);
        //         //     if (handler.status === "success") {
        //         //         resolve(context.data);
        //         //     } else {
        //         //         reject(context.error || Error("broadcat timeout"));
        //         //     }
        //         // } else if (handler.status !== "idle") {
                    
        //         }
        //         i++;
        //     }, 333);
        // });
//let handler = iost.callABI("iost.token", "transfer", ["iost", "form", "to", "1000.000"]);

//handler
//    .onPending(console.log)
//    .onSuccess(console.log)
//    .onFailed(console.log)
//    .send()
//    .listen(); // if not listen, only onPending or onFailed (at sending tx) will be called
