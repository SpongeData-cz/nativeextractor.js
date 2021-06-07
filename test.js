// nodejs -i --eval "$(cat test.js)"

const ne = require('./nativeextractor.js');
const path = require('path');

let results = [];

function test() {
    const ex = new ne.Extractor(
        [
            ["ne", "ne"],
            [path.join(ne.defaultMinersPath, "network_entities.so"), "match_ipv4"],
            [path.join(ne.defaultMinersPath, "network_entities.so"), "match_ipv1337"],
            [path.join(ne.defaultMinersPath, "web_entities.so"), "a_missing_symbol"],
            ["me", "me"],
        ],
        10
    );
    console.log(ex);
    if (!ex.addMiner(path.join(ne.defaultMinersPath, "web_entities.so"), "match_email")) {
        console.log(ex.getLastError());
        return;
    }
    console.log(ex);

    const fpath = "./test/fixtures/web_entities.txt";
    console.log(`Opening ${fpath}`)
    console.log(ex.open(fpath));
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
            console.log(`Closing ${fpath}`);
            ex.unsetStream();
            console.log(`Closed ${fpath}`);
            return;
        }
        ex.next(arguments.callee);
    });
}

test();