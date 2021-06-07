"use strict";

// const ne = require('bindings')('nativeextractor.node');
const os = require('os');
const path = require('path');
process.dlopen(module, __dirname + "/build/Release/nativeextractor.node",
  os.constants.dlopen.RTLD_NOW | os.constants.dlopen.RTLD_GLOBAL);
const ne = module.exports;

const defaultMinersPath = path.join("/", "usr", "lib", "nativeextractor_miners");

/**
 * An abstract stream.
 */
class Stream {

  /**
   * Constructs a new stream with set pointer. Throws an Error if the stream
   * failed.
   *
   * @param {Number} stream the pointer
   */
  constructor(stream) {
    this._stream = stream;
    this.streamCheck(this._stream);
  }

  /**
   * Throws an Error if the stream failed.
   */
  streamCheck() {
    if (!ne.stream_check(this._stream)) {
      throw new Error("Stream failed!");
    }
  }

}

/**
 * Stream from a file.
 */
class FileStream extends Stream {

  /**
   * Constructs a stream from the given file.
   *
   * @param {String} fullpath the path to the file
   */
  constructor(fullpath) {
    const ptr = ne.stream_file_new(fullpath);
    super(ptr);
  }

  /**
   * Frees the file stream.
   */
  destructor() {
    ne.free_file_stream(self.stream);
  }

}

/**
 * Stream from a memory buffer.
 */
class BufferStream extends Stream {

  /**
   * Constructs the stream from the given buffer.
   *
   * @param {String} buffer buffer of memory
   */
  constructor(buffer) {
    const ptr = ne.stream_buffer_new(buffer);
    super(ptr);
  }

  /**
   * Frees the buffer stream.
   */
  destructor() {
    ne.free_buffer_stream(self.stream);
  }

}

/**
 * An extractor with asynchronous mining.
 */
class Extractor {

  /**
   * Constructs an extactor with given miners, batch size and thread count.
   *
   * @param {Array} miners the miners, see addMiner for format
   * @param {Number} batch the batch size for each mining
   * @param {Number} threads how many threads to use (0 will use all threads
   *                         on current machine)
   */
  constructor(miners, batch, threads) {
    const defaults = {
      miners: [],
      batch: 1000,
      threads: 1,
    };
    this.miners = defaults.miners;
    this.batch = batch || defaults.batch;
    this._extractor = ne.create_extractor(threads || defaults.threads);

    miners = miners || defaults.miners;
    for (let miner of miners) {
      if (!this.addMiner(...miner)) {
        console.warn(
          `Couldn't add ${miner[0]}::${miner[1]}: `
          + `${this.getLastError()}.`
        );
      }
    }
  }

  /**
   * Frees the extractor.
   */
  destructor() {
    ne.free_extractor(this._extractor);
  }

  /**
   * Checks the stream and throws an Error if it's not set.
   */
  _checkStream() {
    if (this.stream === undefined) {
      throw new Error("No stream set");
    }
  }

  /**
   * Adds a miner to the extractor.
   *
   * @param {String} so_dir path to file with miners
   * @param {String} symb name of miner
   * @param {String} params arguments for the miner, if any
   *
   * @return true on success
   */
  addMiner(so_dir, symb, params) {
    params = (params === undefined)
      ? ""
      : params;
    if (ne.add_miner_so(
      this._extractor,
      so_dir,
      symb,
      params)
    ) {
      this.miners = [...this.miners, [so_dir, symb, params]];
      return true;
    };
    return false;
  }

  /** Opens new FileStream and sets it to the Extractor.
   *
   * @param {String} path Path to a file
   */
  open(path) {
    const stream = new FileStream(path);
    this.setStream(stream);
  }

  /** Closes previously opened stream via .open() implicitly. */
  close() {
    this.unsetStream();
  }

  /**
   * Sets the stream to mine from.
   *
   * @param {Stream} stream the stream
   *
   * @return this extractor
   */
  setStream(stream) {
    this.stream = stream;
    if (!ne.set_stream(this._extractor, stream._stream)) {
      throw new Error("Cannot set stream!");
    }
    return this;
  }

  /**
   * Unsets any set stream.
   *
   * @return this extractor
   */
  unsetStream() {
    ne.unset_stream(this._extractor);
    this.stream = undefined;
    return this;
  }

  /**
   * Check for EOF in the current stream.
   *
   * @return true if stream at EOF
   */
  eof() {
    this._checkStream();
    return ne.eof(this._extractor);
  }

  /**
   * Asynchronously processes the next batch in the stream.
   *
   * @param {function (error, result)} callback what to call when done
   * @param {Number} batch batch size, optional
   */
  next(callback, batch) {
    if (batch === undefined) {
      batch = this.batch;
    }
    if (callback === undefined) {
      callback = function () { };
    }
    this._checkStream();
    return ne.next(this._extractor, batch, callback);
  }

  /**
   * Returns the last error message.
   *
   * @return the last error message
   */
  getLastError() {
    return ne.get_last_error(this._extractor);
  }

  /**
   * Returns metainfo. Maps labels to {path, miner, label} dicts.
   *
   * @return {Object<String, Object>} the metainfo
   */
  getMeta() {
    const ret = {};
    const dlsyms = ne.dlsymbols(this._extractor);
    for (const i in dlsyms) {
      const entry = dlsyms[i];
      ret[entry.label] = entry;
    }
    return ret;
  }

  /**
   * @param {{ (accumulator, occurrences: Object[], callback?: (err, res) => void) => void }} fn
   * A function executed for each list of mined occurrences. If you want it to be
   * asynchronous, it needs to take three argument, where the last one is a callback.
   * @param {*} initialValue
   * @param {{ (err, res) => void }} callback
   */
  reduce(fn, initialValue, callback) {
    let isFirst = true;
    let acc;

    let handleNext;

    if (fn.length > 2) {
      // Asynchronous version
      handleNext = (err, res) => {
        if (err) {
          callback(err);
          return;
        }

        if (isFirst) {
          acc = (initialValue !== undefined) ? initialValue : res;
          isFirst = false;
        }

        fn(acc, res, (err, retval) => {
          if (err) {
            callback(err);
            return;
          }

          acc = retval;

          if (this.eof()) {
            callback(null, acc);
            return;
          }

          this.next(handleNext);
        });
      };
    } else {
      // Synchronous version
      handleNext = (err, res) => {
        if (err) {
          callback(err);
          return;
        }

        if (isFirst) {
          acc = (initialValue !== undefined) ? initialValue : res;
          isFirst = false;
        }

        try {
          acc = fn(acc, res);
        } catch (e) {
          callback(e);
          return;
        }

        if (this.eof()) {
          callback(null, acc);
          return;
        }

        this.next(handleNext);
      };
    }

    this.next(handleNext);
  }

}

Extractor.extractMeta = function (path) {
  const ret = {};
  const meta = ne.extract_meta_node(path);
  for (const i in meta) {
    const entry = meta[i];
    ret[entry.label] = entry;
  }
  return ret;
}

module.exports = {
  Extractor,
  Stream,
  BufferStream,
  FileStream,
  defaultMinersPath
};
