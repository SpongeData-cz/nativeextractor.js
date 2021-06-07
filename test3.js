// nodejs test3.js

const ne = require('./nativeextractor.js');
const path = require('path');

let results = [];

function test() {
    const ex = new ne.Extractor(
        [
            [path.join(ne.defaultMinersPath, "glob_entities.so"), "match_glob", "*"],
            // [path.join(ne.defaultMinersPath, "web_entities.so"), "match_email"],
        ]
    );
    console.log(ex);

    const fpath = "./test/fixtures/web_entities.txt";
    console.log(`Opening ${fpath}`)
    console.log(ex.open(fpath));
    console.log(`EOF: ${ex.eof()}`);
    if (ex.eof()) {
        return;
    }

    ex.next(function (err, res) {
        console.log("Last error:");
        console.log(ex.getLastError());
        console.log(`Err:`);
        console.log(err);
        console.log(`Results of next:`);
        console.log(res);
        console.log(`EOF: ${ex.eof()}`);
        results = [...results, ...res];
        if (ex.eof()) {
            console.log(`Closing ${fpath}`);
            console.log(ex.close());
            console.log(`Closed ${fpath}`);
            return;
        }
        ex.next(arguments.callee);
    });
}

test();