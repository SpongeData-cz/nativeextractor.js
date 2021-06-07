// nodejs -i --eval "$(cat test4.js)"

const ne = require('./nativeextractor.js');
const path = require('path');

let results = [];

function test() {
    const ex = new ne.Extractor();
    if (!ex.addMiner(path.join(ne.defaultMinersPath, "web_entities.so"), "match_email")) {
        console.log(ex.getLastError());
        return;
    }

    const buffer = "tozja@toz.ja";
    const stream = new ne.BufferStream(buffer);
    console.log(`Processing buffer '${buffer}'`)
    console.log(ex.setStream(stream));
    console.log(`EOF: ${ex.eof()}`);
    if (ex.eof()) {
        return;
    }

    ex.next(function (err, res) {
        console.log(`Results of next:`);
        console.log(res);
        console.log(`EOF: ${ex.eof()}`);
        results = [...results, ...res];
        if (ex.eof()) {
            console.log(`Unsetting stream`);
            console.log(ex.unsetStream());
            console.log(`Unset stream`);
            return;
        }
        ex.next(arguments.callee);
    });
}

test();