// const Transport = require('@ledgerhq/hw-transport');
const TransportU2F = require( "@ledgerhq/hw-transport-u2f");
const TransportNodeHid = require( "@ledgerhq/hw-transport-node-hid-noevents");
const Logs = require( "@ledgerhq/logs");
const Utils = require("./utils");


module.exports = function(appName, transport) {
    // global.fetch = () => {
    //     console.error('required node version >= 10.x')
    //     process.exit(10);
    // };
  const APDU = {
    CLA: 0xE0,
    INS_GET_CONFIGURATION: 0x02,
    INS_GET_PUBLIC_KEY: 0x04,
    INS_SIGN_TRANSACTION: 0x08,
    INS_SIGN_TRX_HASH: 0x16,
  }

  if (Utils.debugging) {
    Logs.listen(console.log);
  }
    
  this.open = async () => {
    let t = null;
    let descriptor = "";

    switch (transport.split(":")[0]) {
      case "u2f":
        t = await TransportU2F.default;
        break;
      case "hid":
        t = await TransportNodeHid.default;
        break;
      }

    if (t === null) {
      throw new Error("Unknown ledger transport - " + transport);
    } else if (! await t.isSupported()) {
      throw new Error("Ledger transport '" + transport + "' not supported on this platform");
    } else {
      this.transport = await t.open(descriptor);
      this.transport.decorateAppAPIMethods(
        this,
        [
          "getConfiguration",
          "getPublicKey",
          "signTransaction"
        ],
        appName
      );
    }
  };
  this.close = async () => {
    await this.transport.close();
  };
  this.getConfiguration = async (path) => {
    const buffer = Utils.bufferFromBip32(path);
    const data = await this.transport.send(
      APDU.CLA,
      APDU.INS_GET_CONFIGURATION,
      0,
      0,
      buffer
    );
    return {
      hasStorage: data[0] != 0,
      version: data[1] + "." + data[2] + "." + data[3]
    }
  };
  this.getPublicKey = async (path, p1, p2) => {
    const buffer = Utils.bufferFromBip32(path);
    const data = await this.transport.send(
      APDU.CLA,
      APDU.INS_GET_PUBLIC_KEY,
      p1 || 0,
      p2 || 0,
      buffer
    );

    if (p2 == 0) {
      return {
        hex: data.slice(0, -3).toString("utf8")
      };
    }
    if (p2 == 1) {
      return {
        base58: data.slice(0, -3).toString("utf8")
      };
    }
    return {
      bin: new Uint8Array(data.slice(0, -3))
    };
  };
  this.signTransaction = async (path, trxHash) => {
       const buffer = Utils.bufferFromBip32(path);
       const data = await this.transport.send(
         APDU.CLA,
         APDU.INS_SIGN_TRX_HASH,
         1,
         0,
         Buffer.concat([buffer, trxHash])
       );
       return data.slice(0, -2);
  };
};

//   /**
//    * Returns public key and ICON address for a given BIP 32 path.
//    * @param path a path in BIP 32 format
//    * @option boolDisplay optionally enable or not the display
//    * @option boolChaincode optionally enable or not the chaincode request
//    * @return an object with a publickey(hexa string), address(string) and 
//    *  (optionally) chaincode(hexa string)
//    * @example
//    * icx.getAddress("44'/4801368'/0'", true, true).then(o => o.address)
//    */
//   getAddress(
//     path: string,
//     boolDisplay?: boolean = false,
//     boolChaincode?: boolean = true
//   ): Promise<{
//     publicKey: string,
//     address: string,
//     chainCode?: string
//   }> {
//     let paths = splitPath(path);
//     let buffer = new Buffer(1 + paths.length * 4);
//     buffer[0] = paths.length;
//     paths.forEach((element, index) => {
//       buffer.writeUInt32BE(element, 1 + 4 * index);
//     });
//     return this.transport
//       .send(
//         0xe0,
//         0x02,
//         boolDisplay ? 0x01 : 0x00, 
//         boolChaincode ? 0x01 : 0x00, 
//         buffer
//       )
//       .then(response => {
//         let result = {};
//         let publicKeyLength = response[0];
//         result.publicKey = response.slice(1, 1 + publicKeyLength).toString("hex");
//         let addressLength = response[1 + publicKeyLength];
//         result.address = response.slice(1 + publicKeyLength + 1, 1 + publicKeyLength + 1 + addressLength);
//         if (boolChaincode) {
//           result.chainCode = response.slice(-32).toString("hex");
//         }
//         return result;
//       });
//   }

//   /**
//    * Signs a transaction and returns signed message given the raw transaction
//    * and the BIP 32 path of the account to sign
//    * @param path a path in BIP 32 format
//    * @param rawTxAscii raw transaction data to sign in ASCII string format
//    * @return an object with a base64 encoded signature and hash in hexa string
//    * @example
//    * icx.signTransaction("44'/4801368'/0'",
//    *     "icx_sendTransaction.fee.0x2386f26fc10000." +
//    *     "from.hxc9ecad30b05a0650a337452fce031e0c60eacc3a.nonce.0x3." +
//    *     "to.hx4c5101add2caa6a920420cf951f7dd7c7df6ca24.value.0xde0b6b3a7640000")
//    *   .then(result => ...)
//    */
//   signTransaction(
//     path: string,
//     rawTxAscii: string
//   ): Promise<{
//     signedRawTxBase64: string,
//     hashHex: string
//   }> {
//     let paths = splitPath(path);
//     let offset = 0;
//     let rawTx = new Buffer(rawTxAscii);
//     let toSend = [];
//     let response;
//     while (offset !== rawTx.length) {
//       let maxChunkSize = offset === 0 ? 150 - 1 - paths.length * 4 - 4: 150;
//       let chunkSize =
//         offset + maxChunkSize > rawTx.length
//           ? rawTx.length - offset
//           : maxChunkSize;
//       let buffer = new Buffer(
//         offset === 0 ? 1 + paths.length * 4 + 4 + chunkSize : chunkSize
//       );
//       if (offset === 0) {
//         buffer[0] = paths.length;
//         paths.forEach((element, index) => {
//           buffer.writeUInt32BE(element, 1 + 4 * index);
//         });
//         buffer.writeUInt32BE(rawTx.length, 1 + 4 * paths.length);
//         rawTx.copy(buffer, 1 + 4 * paths.length + 4, offset, offset + chunkSize);
//       } else {
//         rawTx.copy(buffer, 0, offset, offset + chunkSize);
//       }
//       toSend.push(buffer);
//       offset += chunkSize;
//     }
//     return foreach(toSend, (data, i) =>
//       this.transport
//         .send(0xe0, 0x04, i === 0 ? 0x00 : 0x80, 0x00, data)
//         .then(apduResponse => {
//           response = apduResponse;
//         })
//     ).then(() => {
//       let result = {};
//       // r, s, v are aligned sequencially
//       result.signedRawTxBase64 = 
//         hexToBase64(response.slice(0, 32 + 32 + 1).toString("hex"));
//       result.hashHex = 
//         response.slice(32 + 32 + 1, 32 + 32 + 1 + 32).toString("hex");
//       return result;
//     });
//   }

//   /**
//    * Returns the application configurations such as versions.
//    * @return  major/minor/patch versions of Icon application
//    */
//   getAppConfiguration(): Promise<{
//     majorVersion: number,
//     minorVersion: number,
//     patchVersion: number
//   }> {
//     return this.transport
//       .send(0xe0, 0x06, 0x00, 0x00)
//       .then(response => {
//         let result = {};
//         result.majorVersion = response[0];
//         result.minorVersion = response[1];
//         result.patchVersion = response[2];
//         return result;
//       });
//   }

//   /**
//    * Sets the given key as the test purpose private key corresponding to 
//    * "\0'" of BIP 32 path just for test purpose. After calling this function,
//    * all functions with "\0'" path works based on this private key. 
//    * REMARK: Test purpose only such as verifying signTransaction function. 
//    * @param privateKeyHex private key in hexadecimal string format
//    * @example
//    * icx.setTestPrivateKey("23498dc21b9ee52e63e8d6566e0911ac255a38d3fcbc68a51e6b298520b72d6e")
//    *   .then(result => ...)
//    * icx.getAddress("0'", false, false).then(o => o.address)
//    */
//   setTestPrivateKey(privateKeyHex: string) {
//     let data = new Buffer(32);
//     for (let i = 0; i < privateKeyHex.length; i += 2) {
//       data[i / 2] = parseInt(privateKeyHex.substr(i, 2), 16);
//     }
//     return this.transport.send(0xe0, 0xff, 0x00, 0x00, data).then();
//   }
// }
