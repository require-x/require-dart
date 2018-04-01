import 'package:require/require.dart';

void main() {
  var semver = require('semver');
  print(semver.valid('v1.0.0')); // 1.0.0
}
