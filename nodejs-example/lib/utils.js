const BipPath = require("bip32-path");

// type Defer<T> = {
//     promise: Promise<T>,
//     resolve: T => void,
//     reject: any => void
// };
  
// export function defer<T>(): Defer<T> {
//     let resolve, reject;
//     let promise = new Promise(function(success, failure) {
//       resolve = success;
//       reject = failure;
//     });
//     if (!resolve || !reject) throw "defer() error"; // this never happens and is just to make flow happy
//     return { promise, resolve, reject };
//   }
  
//   // TODO use bip32-path library
//   export function splitPath(path: string): number[] {
//     let result = [];
//     let components = path.split("/");
//     components.forEach(element => {
//       let number = parseInt(element, 10);
//       if (isNaN(number)) {
//         return; // FIXME shouldn't it throws instead?
//       }
//       if (element.length > 1 && element[element.length - 1] === "'") {
//         number += 0x80000000;
//       }
//       result.push(number);
//     });
//     return result;
//   }
  
//   // TODO use async await
  
//   export function eachSeries<A>(arr: A[], fun: A => Promise<*>): Promise<*> {
//     return arr.reduce((p, e) => p.then(() => fun(e)), Promise.resolve());
//   }
  
//   export function foreach<T, A>(
//     arr: T[],
//     callback: (T, number) => Promise<A>
//   ): Promise<A[]> {
//     function iterate(index, array, result) {
//       if (index >= array.length) {
//         return result;
//       } else
//         return callback(array[index], index).then(function(res) {
//           result.push(res);
//           return iterate(index + 1, array, result);
//         });
//     }
//     return Promise.resolve().then(() => iterate(0, arr, []));
//   }
  
//   export function doIf(
//     condition: boolean,
//     callback: () => any | Promise<any>
//   ): Promise<void> {
//     return Promise.resolve().then(() => {
//       if (condition) {
//         return callback();
//       }
//     });
//   }
  
//   export function asyncWhile<T>(
//     predicate: () => boolean,
//     callback: () => Promise<T>
//   ): Promise<Array<T>> {
//     function iterate(result) {
//       if (!predicate()) {
//         return result;
//       } else {
//         return callback().then(res => {
//           result.push(res);
//           return iterate(result);
//         });
//       }
//     }
//     return Promise.resolve([]).then(iterate);
//   }
  
//   export function hexToBase64(hexString: string) {
//     return btoa(hexString.match(/\w{2}/g).map(function(a) {
//         return String.fromCharCode(parseInt(a, 16));
//     }).join(""));
//   }  

const debugging = () => {
  return process.env.NODE_ENV === "development";
};

const delay = (ms) => {
  return new Promise(function(resolve) {
      setTimeout(resolve, ms || 1000);
  });
};

const fail = (e) => {
  console.error("App crashed (" + e + ")");
  if (e instanceof Error) {
    console.error(e.stack);
  }
  process.exit(1);
};


const bufferFromBip32 = (path) => {
  const paths = path ? BipPath.fromString(path).toPathArray() : [];
  const result = Buffer.alloc(1 + paths.length * 4);
  result[0] = paths.length;
  paths.forEach((element, index) => {
    result.writeUInt32BE(element, 1 + 4 * index);
  });
  return result;
};

const callAsync = (func, args) => {
  return new Promise((resolve, reject) => {
    func().then(function() {
      if (args === undefined) {
        args = Object.values(arguments);
      } else if (Array.isArray(args)) {
        args = Object.values(arguments).concat(args);
      } else {
        args = Object.values(arguments).concat([args]);
      }
      resolve(args);
    }).catch(reject);
  });
};

const bufferToHex = (buffer) => {
  return Array
    .from(new Uint8Array(buffer))
    .map(b => b.toString(16).padStart (2, "0"))
    .join("");
};

const hexStringToArray = (hexStr) => {
//    var result = [];
//    while (str.length >= 8) {
//      result.push(parseInt(str.substring(0, 8), 16));
//      str = str.substring(8, str.length);
//    }
//    return result;
  if (!hexStr) {
    return new Uint8Array();
  }
  var array = [];
  for (var i = 0, length = hexStr.length; i < length; i += 2) {
    array.push(parseInt(hexStr.substr(i, 2), 16));
  }
  return new Uint8Array(array);
}

const arrayToHexString = (uint8arr) => {
//    let result = "";
//    let z;
//    for (var i = 0; i < arr.length; i++) {
//      let str = arr[i].toString(16);
//      z = 8 - str.length + 1;
//      str = Array(z).join("0") + str;
//      result += str;
//    }
//    return result;
  if (!uint8arr) {
    return "";
  }

  let hexStr = '';
  for (let i = 0; i < uint8arr.length; i++) {
    var hex = (uint8arr[i] & 0xff).toString(16);
    hex = (hex.length === 1) ? "0" + hex : hex;
    hexStr += hex;
  }

  return hexStr.toUpperCase();
}

module.exports = { 
  debugging,
  delay,
  fail,
  bufferFromBip32,
  callAsync,
  bufferToHex,
  hexStringToArray,
  arrayToHexString
};


