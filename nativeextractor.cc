#include <napi.h>

extern "C" {
  #include <nativeextractor/extractor.h>
  #include <nativeextractor/miner.h>
  #include <nativeextractor/occurrence.h>
  #include <nativeextractor/stream.h>
}

#define exportfn(fn) exports.Set(Napi::String::New(env, #fn), \
                                 Napi::Function::New(env, fn));

/**
 * Creates a new file stream.
 * 
 * @param info[0] the fullpath of the target file
 * @return pointer to the stream
 */
Napi::Value stream_file_new(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Argument must be a string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string fullpath = info[0]
    .As<Napi::String>()
    .Utf8Value();

  stream_file_c *sf = stream_file_c_new(fullpath.data());

  if (sf->stream.state_flags & STREAM_FAILED) {
    DESTROY(sf);
    Napi::Error::New(env, "Stream failed").ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Number::New(env, (long) sf);
}

/**
 * Returns true if the stream hasn't failed.
 * 
 * @param info[0] a pointer to the stream
 * @return true if the stream hasn't failed
 */
Napi::Value stream_check(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Argument must be a pointer to stream")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  stream_c *stream = (stream_c *) info[0].As<Napi::Number>().Int64Value();

  return Napi::Boolean::New(env, !(stream->state_flags & STREAM_FAILED));
}

/**
 * Destroys a file stream.
 * 
 * @param info[0] a pointer to the stream
 */
Napi::Value free_file_stream(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Argument must be a pointer to stream")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  stream_file_c *stream = (stream_file_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  DESTROY(stream);

  return env.Undefined();
}

/**
 * Destroys a buffer stream.
 * 
 * @param info[0] a pointer to the stream
 */
Napi::Value free_buffer_stream(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Argument must be a pointer to stream")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  stream_buffer_c *stream = (stream_buffer_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  DESTROY((stream_c *) stream);

  return env.Undefined();
}

/**
 * Creates a new buffer stream.
 * 
 * @param info[0] the contents of the buffer
 * @return pointer to the stream
 */
Napi::Value stream_buffer_new(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "Argument must be a string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string buffer = info[0]
    .As<Napi::String>()
    .Utf8Value();

  stream_buffer_c *sb = stream_buffer_c_new(
    (const uint8_t *) strdup(buffer.data()),
    buffer.length()
  );

  if (sb->stream.state_flags & STREAM_FAILED) {
    DESTROY((stream_c *) sb);
    Napi::Error::New(env, "Stream failed").ThrowAsJavaScriptException();
    return env.Null();
  }

  return Napi::Number::New(env, (long) sb);
}

/**
 * Creates a new extractor
 * 
 * @param info[0] the number of threads to use
 * @return pointer to the extractor
 */
Napi::Value create_extractor(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "Argument must be a number")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  int threads = info[0].As<Napi::Number>().Int32Value();

  miner_c **m = (miner_c **) malloc(sizeof(miner_c *));
  m[0] = NULL;
  extractor_c *extractor = extractor_c_new(threads, m);

  return Napi::Number::New(env, (long) extractor);
}

/**
 * Adds a miner an extractor.
 * 
 * @param info[0] the extractor
 * @param info[1] the fullpath of the *.so file
 * @param info[2] the symbol of the miner
 * @param info[3] the params of the miner
 * 
 * @return true on success
 */
Napi::Value add_miner_so(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 4) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[1].IsString() || !info[2].IsString() || !info[3].IsString()) {
    Napi::TypeError::New(env, "so_dir, symb and params must be strings")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();
  std::string so_dir = info[1]
      .As<Napi::String>()
      .Utf8Value();
  std::string symb = info[2]
      .As<Napi::String>()
      .Utf8Value();
  std::string params = info[3]
      .As<Napi::String>()
      .Utf8Value();

  return Napi::Boolean::New(
    env,
    extractor->add_miner_so(extractor, so_dir.data(), symb.data(), (void *) strdup(params.data()))
  );
}

/**
 * Returns true if stream in extractor is at EOF.
 * 
 * @param info[0] pointer to extractor
 * @return true if stream in extractor is at EOF
 */
Napi::Value eof(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  return Napi::Boolean::New(
    env,
    (extractor->stream->state_flags) & STREAM_EOF
  );
}

/**
 * Worker for the `next` method of the extractor, so it may run asynchronously.
 */
class NextWorker : public Napi::AsyncWorker {

  public:
    NextWorker(
      extractor_c *extractor,
      unsigned int batch,
      Napi::Function callback
    ) : Napi::AsyncWorker(callback, "NextWorker"),
        extractor(extractor),
        batch(batch) {}

    ~NextWorker() {}

    void Execute() {
      /*
       * no JavaScript allowed here
       */
      occurrences = extractor->next(extractor, batch);
    }

    /**
     * Creates all the occurrences as JavaScript objects and calls the callback
     * with an array of them as the parameter.
     */
    void OnOK() {
      Napi::Env env = Env();
      Napi::HandleScope scope(env);
      Napi::Array results = Napi::Array::New(env);

      occurrence_t **pres = occurrences;
      while (*pres) {
        Napi::Object occurrence = Napi::Object::New(env);
        occurrence.Set(
          Napi::String::From(env, "pos"),
          Napi::Number::From(env, (*pres)->pos)
        );
        occurrence.Set(
          Napi::String::From(env, "len"),
          Napi::Number::From(env, (*pres)->len)
        );
        occurrence.Set(
          Napi::String::From(env, "label"),
          Napi::String::From(env, (*pres)->label)
        );
        char *s = (char *) malloc((*pres)->len + 1);
        strncpy(s, ((*pres)->str), (*pres)->len);
        s[(*pres)->len] = '\0';
        occurrence.Set(
          Napi::String::From(env, "value"),
          Napi::String::From(env, s)
        );
        occurrence.Set(
          Napi::String::From(env, "prob"),
          Napi::Number::From(env, (*pres)->prob)
        );
        free(s);

        results.Set((uint32_t) results.Length(), occurrence);

        free(*pres);
        ++pres;
      }

      free(occurrences);

      Callback().Call({
        env.Undefined(), // err
        results
      });
    }

  private:
    extractor_c *extractor;
    unsigned int batch;
    occurrence_t **occurrences;

};

/**
 * Asynchronously mines entities out of set stream.
 * 
 * @param info[0] the extractor
 * @param info[1] batch size
 * @param info[2] callback
 */
Napi::Value next(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 3) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Second argument must be batch size")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Third argument must be a function")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();
  unsigned int batch = info[1].As<Napi::Number>().Uint32Value();
  Napi::Function callback = info[2].As<Napi::Function>();

  NextWorker* nextWorker = new NextWorker(extractor, batch, callback);
  nextWorker->Queue();

  return env.Undefined();
}

/**
 * Returns the last error.
 * 
 * @param info[0] extractor
 * @return the last error
 */
Napi::Value get_last_error(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  return Napi::String::From<const char *>(
    env,
    extractor->get_last_error(extractor)
  );
}

/**
 * Sets the stream to an extractor.
 * 
 * @param info[0] the extractor
 * @param info[1] the stream
 * 
 * @return true on success
 */
Napi::Value set_stream(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "Second argument must be stream address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  stream_c *stream = (stream_c *) info[1]
      .As<Napi::Number>()
      .Int64Value();

  return Napi::Boolean::New(env, extractor->set_stream(extractor, stream));
}

/**
 * Unsets a set stream.
 * 
 * @param info[0] the extractor
 */
Napi::Value unset_stream(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  extractor->unset_stream(extractor);

  return env.Undefined();
}

/**
 * Returns the dlsymbols.
 * 
 * @param info[0] extractor
 * @return the dlsymbols
 */
Napi::Value dlsymbols(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be extractor address")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  extractor_c *extractor = (extractor_c *) info[0]
      .As<Napi::Number>()
      .Int64Value();

  Napi::Array list = Napi::Array::New(env);

  dl_symbol_t **dlsyms = extractor->dlsymbols;

  while (*dlsyms) {
    const char *path = (*dlsyms)->ldpath;
    const char **meta = (*dlsyms)->meta;
    while (*meta) {
      const char *miner = meta[0];
      const char *label = meta[1];
      Napi::Object entry = Napi::Object::New(env);
      entry.Set(
        Napi::String::From(env, "path"),
        Napi::String::From(env, path)
      );
      entry.Set(
        Napi::String::From(env, "miner"),
        Napi::String::From(env, miner)
      );
      entry.Set(
        Napi::String::From(env, "label"),
        Napi::String::From(env, label)
      );
      list.Set((uint32_t) list.Length(), entry);
      meta += 2;
    }
    ++dlsyms;
  }

  return list;
}

/**
 * Extracts meta from miner so file.
 * 
 * @param info[0] path to so file
 * @return meta info
 */
Napi::Value extract_meta_node(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "First argument must be a path")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string path = info[0]
    .As<Napi::String>()
    .Utf8Value();

  Napi::Array list = Napi::Array::New(env);

  char **meta = extract_meta(path.data());

  while (*meta) {
    const char *miner = meta[0];
    const char *label = meta[1];
    Napi::Object entry = Napi::Object::New(env);
    entry.Set(
      Napi::String::From(env, "path"),
      Napi::String::From(env, path)
    );
    entry.Set(
      Napi::String::From(env, "miner"),
      Napi::String::From(env, miner)
    );
    entry.Set(
      Napi::String::From(env, "label"),
      Napi::String::From(env, label)
    );
    list.Set((uint32_t) list.Length(), entry);
    meta += 2;
  }

  // free_meta(meta);

  return list;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exportfn(create_extractor);
  exportfn(add_miner_so);
  exportfn(eof);
  exportfn(next);
  exportfn(get_last_error);
  exportfn(stream_buffer_new);
  exportfn(stream_file_new);
  exportfn(free_buffer_stream);
  exportfn(free_file_stream);
  exportfn(set_stream);
  exportfn(unset_stream);
  exportfn(stream_check);
  exportfn(dlsymbols);
  exportfn(extract_meta_node);
  return exports;
}

NODE_API_MODULE(addon, Init)
