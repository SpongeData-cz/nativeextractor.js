// nodejs -i --eval "$(cat test5.js)"

const ne = require('./nativeextractor.js');
const os = require('os');
const path = require('path');

process.dlopen(module, "build/Release/nativeextractor.node",
  os.constants.dlopen.RTLD_NOW | os.constants.dlopen.RTLD_GLOBAL);
const neb = module.exports;

let results = [];

function test() {
    const ex = new ne.Extractor();

    if (!ex.addMiner(path.join(ne.defaultMinersPath, "web_entities.so"), "match_email")) {
        console.log(ex.getLastError());
        return;
    }

    console.log(ex.getMeta());
    console.log(ex.miners);
}

test();