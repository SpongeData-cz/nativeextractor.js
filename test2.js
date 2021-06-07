// nodejs test2.js

const ne = require('./nativeextractor.js');
const path = require('path');
let finished = 0;
let interval;
const results = [];

function test(extractors) {
    if (extractors === undefined) {
        extractors = 1;
    }
    const ex = []
    for (let i = 0; i < extractors; ++i) {
        ex.push(new ne.Extractor(
            [
                // [path.join(ne.defaultMinersPath, "date_entities.so"), "match_date"],
                [path.join(ne.defaultMinersPath, "web_entities.so"), "match_email"],
            ],
            10000000,
            1
        ));
    }

    const fpath = "./test/fixtures/web_entities.txt";
    console.log(`Running ${ex.length} extractors `
                 + `with ${ex[0].miners.length} miners on ${fpath}.`);

    function runExtractor(e) {
        e.open(fpath);
        e.next(function (err, res) {
            results.push(...res);
            if (e.eof()) {
                e.close();
                finished += 1;
                console.log(`Extractor ${finished} finished.`);
                return;
            }
            e.next(arguments.callee);
        });
    }

    for (let e of ex) {
        runExtractor(e);
    }

    console.log("Extractors dispatched.");

    interval = setInterval(() => {
        if (finished === extractors) {
            console.log("Finished.");
            console.log(results.length);
            clearInterval(interval);
        }
    }, 50);
}

test(12);