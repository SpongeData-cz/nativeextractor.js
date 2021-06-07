{
  "targets": [
    {
      "target_name": "nativeextractor",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
        "nativeextractor.cc",
        # "<!@(python -c \"import os; print '\n'.join(['%s' % x for x in os.listdir('../../') if x[-2:] in ('.c', '.h') and 'main' not in x]) + '\nnativeextractor.cc'\")",
        "./nativeextractor/src/csv_parser.c",
        "./nativeextractor/src/extractor.c",
        "./nativeextractor/src/miner.c",
        "./nativeextractor/src/ner.c",
        "./nativeextractor/src/occurrence.c",
        "./nativeextractor/src/pair.c",
        "./nativeextractor/src/patricia.c",
        "./nativeextractor/src/patricia_miner.c",
        "./nativeextractor/src/stream.c",
        "./nativeextractor/src/unicode.c",
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        './nativeextractor/src/',
        './nativeextractor/src/bindings',
        './nativeextractor/include',
        '/usr/include/glib-2.0',
        '/usr/lib/x86_64-linux-gnu/glib-2.0/include',
      ],
      "libraries": [
        "-lglib-2.0",
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'extra_link_args': [ '-rdynamic' ],
    }
  ]
}
