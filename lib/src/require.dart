library require;

import 'dart:io';
import 'dart:async';
import 'dart:mirrors';
import 'dart:convert';
import 'package:crypto/crypto.dart';
import 'dart-ext:/usr/local/lib/requiredart';
import 'Undefined.dart';

void load(String sourceCode) native "Load";
dynamic get(String Key, [int ptr = -1]) native "Get";
dynamic call(int ptr, List args, [int thisVal = -1, bool ret = true]) native "Call";
dynamic toString(int ptr) native "ToString";
void init() native "Init";
void cleanup() native "Cleanup";
void startLoop() native "StartLoop";
void runJerryJobsNative() native "RunJerryJobs";

bool ready = false;
int moduleCount = -1;

Function nativeCall = call;
Function nativeToString = toString;

Future loop() async {
  startLoop();
}

Future runJerryJobs() async {
  runJerryJobsNative();
}

Future toFuture(int ptr) {
  Object proxy = new Object(ptr);

  StreamController controller = new StreamController();

  proxy.then.apply(proxy, [(dynamic v) {
    controller.add(v);
  }, (dynamic e) {
    controller.addError(e);
  }], false);

  return controller.stream.first;
}

@proxy
class Object {
  int ptr;

  Object(this.ptr);

  operator [](String s) => get(s, ptr);

  dynamic noSuchMethod(Invocation invocation) {
    if (invocation.isGetter) {
      return get(MirrorSystem.getName(invocation.memberName), ptr);
    } else if (invocation.isMethod) {
      dynamic val = get(MirrorSystem.getName(invocation.memberName), ptr);
      if (!(val is Object)) {
        throw new UnsupportedError(
            '${MirrorSystem.getName(invocation.memberName)} is not a function');
      }

      dynamic ret = nativeCall(val.ptr, invocation.positionalArguments, ptr);

      loop();
      runJerryJobs();

      return ret;
    }

    return null;
  }

  dynamic call(
      [dynamic arg0 = undefined,
      dynamic arg1 = undefined,
      dynamic arg2 = undefined,
      dynamic arg3 = undefined,
      dynamic arg4 = undefined,
      dynamic arg5 = undefined,
      dynamic arg6 = undefined,
      dynamic arg7 = undefined,
      dynamic arg8 = undefined,
      dynamic arg9 = undefined]) {
    dynamic ret = nativeCall(
        ptr, [arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9]);

    loop();
    runJerryJobs();

    return ret;
  }

  dynamic apply(Object thisArg, [List args = const [], bool doRet = false]) {
    dynamic ret = nativeCall(ptr, args, thisArg.ptr, doRet);

    loop();
    runJerryJobs();

    return ret;
  }

  dynamic toString() {
    return nativeToString(ptr);
  }
}

dynamic require(String path) {
  if (!ready) {
    init();
    ready = true;
  }

  moduleCount++;

  new Directory('.jxcore-cache').createSync();

  if (!path.startsWith('.')) {
    var pkg = JSON
        .decode(new File('node_modules/$path/package.json').readAsStringSync());
    path = 'node_modules/$path/${pkg['main']}';
  }

  String hash = sha1.convert(new File(path).readAsBytesSync()).toString();

  File cached = new File('.jxcore-cache/$hash.js');
  if (!cached.existsSync()) {
    ProcessResult result = Process.runSync('webpack', [
      path,
      '--config=/usr/local/etc/require-dart.webpack.config.js',
      '--output=.jxcore-cache/$hash.js',
      '--output-library=module_$hash',
      '--output-library-target=var'
    ]);

    if (result.exitCode != 0) {
      stdout.add(result.stdout);
      stderr.add(result.stderr);
      throw new Exception('webpack failed: ${result.exitCode}');
    }
  }

  load(cached.readAsStringSync());

  var val = get('module_$hash');

  loop();
  runJerryJobs();

  return val;
}
