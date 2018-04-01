# require-dart [![Build Status](https://travis-ci.org/require-x/require-dart.svg?branch=master)](https://travis-ci.org/require-x/require-dart)
Use JS modules in Dart.

## Why?
Because I can.

## Installation
First, install `libuv`.
On ubuntu, run the following.
```bash
$ sudo apt install libuv-dev
```

Add the following to your `pubspec.yaml`
```yaml
dependencies:
  require:
    git:
      url: git://github.com/require-x/require-dart.git
      ref: master
```

Then you have to clone and install the C++ library.
```bash
$ git clone --recursive https://github.com/require-x/require-dart.git
$ cd require-dart
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

Then you can use the library, look at `example/` for examples.

## Type conversions

### Dart -> JS
| Dart | JS |
| --- |---|
| `String` | `String` |
| `num` | `Number` |
| `null` | `null` |
| `Undefined` | `undefined` |
| `bool` | `Boolean` |
| `ByteBuffer` | `ArrayBuffer` |
| `Int8List` | `Int8Array` |
| `..` | `...` |
| `Int64List` | `Int64Array` |
| `Float32List` | `Float32Array` |
| `Float64List` | `Float64Array` |
| `Closure` | `Function` |
| `List` | `Array` |
| `Map` | `Object ` |

### JS -> Dart
| JS | Dart |
| --- |---|
| `undefined` | `Undefined` |
| `null` | `null` |
| `bool` | `Boolean` |
| `Number` | `double` |
| `String` | `String` |
| `Array` | `List` |
| `ArrayBuffer` | `ByteBuffer` |
| `Int8Array` | `Int8List` |
| `..` | `...` |
| `Int64Array` | `Int64List` |
| `Float32Array` | `Float32List` |
| `Float64Array` | `Float64List` |
| `Function` | `Object` |
| `Promise` | `Future` |
| `Object` | `Object` |