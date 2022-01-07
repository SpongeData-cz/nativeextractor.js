const assert = require("assert");
const ne = require("../nativeextractor.js");
const path = require("path");

const doTest = (extractor, buffer, resultCount, cb) => {
  const stream = new ne.BufferStream(buffer);
  let results = [];
  extractor.setStream(stream);
  if (extractor.eof()) {
    return cb(null);
  }
  extractor.next(function (err, res) {
    if (err) {
      extractor.unsetStream();
      return cb(err);
    }
    results = [...results, ...res];
    if (extractor.eof()) {
      extractor.unsetStream();
      assert.equal(results.length, resultCount);
      return cb(null);
    }
    extractor.next(arguments.callee);
  });
};

describe("nativeextractor.js", function () {

  describe("#constructor", function() {

    it("should create an extractor", function () {
      assert.ok(new ne.Extractor());
    });

    it("shouldn't fail when initiated with invalid miners", function () {
      assert.ok(
        new ne.Extractor([
          ["ne", "ne"],
          [path.join(ne.defaultMinersPath, "network_entities.so"), "match_ipv4"],
          [path.join(ne.defaultMinersPath, "network_entities.so"), "match_ipv1337"],
          [path.join(ne.defaultMinersPath, "web_entities.so"), "a_missing_symbol"],
          ["me", "me"],
        ])
      );
    });

  });

  describe("#next", function () {

    it("should mine correctly", function (done) {
      const buffer = "123456789 lolkamo";
      const ex = new ne.Extractor();
      ex.addMiner(
        path.join(ne.defaultMinersPath, "glob_entities.so"),
        "match_glob",
        "?????????"
      );
      doTest(ex, buffer, 1, (err) => {
        if (err) {
          return done(err);
        }
        ex.addMiner(
          path.join(ne.defaultMinersPath, "glob_entities.so"),
          "match_glob",
          "lolkamo"
        );
        doTest(ex, buffer, 2, done);
      });
    });

  });

  describe("#setFlags, #unsetFlags", function () {

    it("should set flags correctly", function () {
      const ex = new ne.Extractor();
      assert(
        ex.setFlags(ne.Extractor.NO_ENCLOSED_OCCURRENCES)
          ==
        ne.Extractor.NO_ENCLOSED_OCCURRENCES
      );
      assert(
        ex.setFlags(ne.Extractor.SORT_RESULTS)
          ==
        ne.Extractor.NO_ENCLOSED_OCCURRENCES | ne.Extractor.SORT_RESULTS
      );
      assert(
        ex.unsetFlags(ne.Extractor.SORT_RESULTS)
          ==
        ne.Extractor.NO_ENCLOSED_OCCURRENCES
      );
    });

    it("should mine according to flags", function (done) {
      const ex = new ne.Extractor(
        [
          [path.join(ne.defaultMinersPath, "glob_entities.so"), "match_glob", "123"],
          [path.join(ne.defaultMinersPath, "glob_entities.so"), "match_glob", "456"],
          [path.join(ne.defaultMinersPath, "glob_entities.so"), "match_glob", "123 456"],
        ]
      );
      const buffer =  "123 456";
    
      doTest(ex, buffer, 3, (err) => {
        if (err) {
          return done(err);
        }
        ex.setFlags(ne.Extractor.NO_ENCLOSED_OCCURRENCES);
        doTest(ex, buffer, 1, (err) => {
          if (err) {
            return done(err);
          }
          done(null);
        });
      });
    });

    it("should match trailing wildcards correctly", function (done) {
      const buffer = "http://2432.spongedata.cz";
      const ex = new ne.Extractor();
      ex.addMiner(
        path.join(ne.defaultMinersPath, "glob_entities.so"),
        "match_glob",
        "????"
      );
      doTest(ex, buffer, 2, done);
    });

  });

});