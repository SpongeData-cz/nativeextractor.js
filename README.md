# nativeextractor.js
> This is official (NativeExtractor)[https://github.com/SpongeData-cz/nativeextractor] binding for Node.js

<p align="center"><img src="https://raw.githubusercontent.com/SpongeData-cz/nativeextractor/main/logo.svg" width="400" /></p>

![Python Logo](logo_nodejs.png)


# Installation
## Requirements
* Node.JS >=10
* `npm`
* `build-essential` (gcc, make)
* `libglib2.0`, `libglib2.0-dev`

We recommend to use virtual environments.
```bash
sudo pip install nodeenv
nodeenv myproject
source myproject/bin/activate
```

## Instant npm solution
**TODO:**
```npm install nativeextractor.js```

## Manual
* Clone the repo
`git clone --recurse-submodules https://github.com/SpongeData-cz/nativeextractor-js.git`

* Install via `npm`
    ```bash
    npm install ./nativeextractor.js/
    ```

# Typical usage

```js
const ne = require('nativeextractor.js');
const path = require('path');

// Accumulated results
let results = [];

function test() {
    // Instantiate Extractor
    const ex = new ne.Extractor();
    // Add naive email miner - see src/example/naive_email_miner.c for details
    if (!ex.addMiner(path.join(ne.defaultMinersPath, "naive_email_miner.so"), "match_email_naive")) {
        console.log(ex.getLastError());
        return;
    }
    // String to extract on
    const buffer = "some@email.org dead beaf";

    // Create buffer stream (from memory)
    const stream = new ne.BufferStream(buffer);

    // Set the stream to the extractor
    ex.setStream(stream);

    // Check if stream is already on its end
    if (ex.eof()) {
        return;
    }

    // Pick next batch of result
    ex.next(function (err, res) {
        // Print found results
        console.log(res);
        results = [...results, ...res];
        if (ex.eof()) {
            // Unset stream for sure
            ex.unsetStream();
            return;
        }
        // Asynchronous recursion
        ex.next(arguments.callee);
    });
}

test();
```