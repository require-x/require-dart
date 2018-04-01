#include <string.h>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <uv.h>
#include <dart_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jerryscript.h>
#include <jxcore.h>

// Forward declaration of ResolveName function.
Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope);

// The name of the initialization function is the extension name followed
// by _Init.
DART_EXPORT Dart_Handle requiredart_Init(Dart_Handle parent_library) {
  if (Dart_IsError(parent_library)) return parent_library;

  Dart_Handle result_code =
      Dart_SetNativeResolver(parent_library, ResolveName, nullptr);
  if (Dart_IsError(result_code)) return result_code;

  return Dart_Null();
}

Dart_Handle HandleError(Dart_Handle handle) {
  if (Dart_IsError(handle)) Dart_PropagateError(handle);
  return handle;
}

void Init(Dart_NativeArguments arguments) {
  init();
}

void Cleanup(Dart_NativeArguments arguments) {
  cleanup();
}

void Load(Dart_NativeArguments arguments) {
  const char *srcCode;
  HandleError(Dart_StringToCString(Dart_GetNativeArgument(arguments, 0), &srcCode));

  jerry_release_value(run(const_cast<char *>(srcCode), strlen(srcCode)));
}

Dart_Handle ResolveToDart(jerry_value_t value) {
  if (jerry_value_is_undefined(value)) {
    Dart_Handle type = HandleError(Dart_GetType(Dart_RootLibrary(), Dart_NewStringFromCString("Undefined"), 0, nullptr));
    Dart_Handle dartUndefined = HandleError(Dart_New(type, Dart_Null(), 0, nullptr));

    return dartUndefined;
  } else if (jerry_value_is_null(value)) {
    return HandleError(Dart_Null());
  } else if (jerry_value_is_boolean(value)) {
    return HandleError(Dart_NewBoolean(jerry_get_boolean_value(value)));
  } else if (jerry_value_is_number(value)) {
    return HandleError(Dart_NewDouble(jerry_get_number_value(value)));
  } else if (jerry_value_is_string(value)) {
    jerry_length_t length = jerry_get_utf8_string_length(value);
    char *str = static_cast<char *>(malloc(length));
    jerry_string_to_utf8_char_buffer(value, reinterpret_cast<jerry_char_t *>(str), length);
    str[length] = 0;

    return HandleError(Dart_NewStringFromCString(str));
  } else if (jerry_value_is_array(value)) {
    uint32_t length = jerry_get_array_length(value);
    Dart_Handle dartList = Dart_NewList(length);

    for (uint32_t i = 0; i < length; i++) {
      jerry_value_t ival = jerry_get_property_by_index(value, i);
      Dart_ListSetAt(dartList, i, ResolveToDart(ival));
      jerry_release_value(ival);
    }

    return dartList;
  } else if (jerry_value_is_typedarray(value)) {
    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t arrayLength = jerry_get_typedarray_length(value);
    jerry_value_t buffer = jerry_get_typedarray_buffer(value, &byteOffset, &byteLength);
    jerry_length_t length = jerry_get_arraybuffer_byte_length(buffer);
    uint8_t *buf = static_cast<uint8_t *>(malloc(length));

    jerry_length_t read = jerry_arraybuffer_read(buffer, 0, buf, arrayLength);

    jerry_typedarray_type_t type = jerry_get_typedarray_type(value);
    Dart_TypedData_Type dartType = Dart_TypedData_kInvalid;
    if (type == JERRY_TYPEDARRAY_INT8) {
      dartType = Dart_TypedData_kInt8;
    } else if (type == JERRY_TYPEDARRAY_UINT8) {
      dartType = Dart_TypedData_kUint8;
    } else if (type == JERRY_TYPEDARRAY_UINT8CLAMPED) {
      dartType = Dart_TypedData_kUint8Clamped;
    } else if (type == JERRY_TYPEDARRAY_INT16) {
      dartType = Dart_TypedData_kInt16;
    } else if (type == JERRY_TYPEDARRAY_UINT16) {
      dartType = Dart_TypedData_kUint16;
    } else if (type == JERRY_TYPEDARRAY_INT32) {
      dartType = Dart_TypedData_kInt32;
    } else if (type == JERRY_TYPEDARRAY_UINT32) {
      dartType = Dart_TypedData_kUint32;
    } else if (type == JERRY_TYPEDARRAY_FLOAT32) {
      dartType = Dart_TypedData_kFloat32;
    } else if (type == JERRY_TYPEDARRAY_FLOAT64) {
      dartType = Dart_TypedData_kFloat64;
    }

    Dart_Handle typedData = HandleError(Dart_NewExternalTypedData(dartType, buf, arrayLength));

    jerry_release_value(buffer);

    return typedData;
  } else if (jerry_value_is_arraybuffer(value)) {
    jerry_length_t length = jerry_get_arraybuffer_byte_length(value);
    uint8_t *buf = static_cast<uint8_t *>(malloc(length));

    jerry_length_t read = jerry_arraybuffer_read(value, 0, buf, length);

    return HandleError(Dart_NewByteBuffer(HandleError(Dart_NewExternalTypedData(Dart_TypedData_kUint8, buf, read))));
  } else if (jerry_value_is_function(value)) {
    Dart_Handle args[1];
    args[0] = Dart_NewInteger(reinterpret_cast<uint32_t>(value));

    Dart_Handle type = HandleError(Dart_GetType(Dart_RootLibrary(), Dart_NewStringFromCString("Object"), 0, nullptr));
    Dart_Handle dartProxy = HandleError(Dart_New(type, Dart_Null(), 1, args));

    return dartProxy;
  } else if (jerry_value_is_promise(value)) {
    Dart_Handle args[1];
    args[0] = HandleError(Dart_NewInteger(reinterpret_cast<uint32_t>(value)));

    return HandleError(Dart_Invoke(Dart_RootLibrary(), Dart_NewStringFromCString("toFuture"), 1, args));
  } else if (jerry_value_is_object(value)) {
    Dart_Handle args[1];
    args[0] = Dart_NewInteger(reinterpret_cast<uint32_t>(value));

    Dart_Handle type = HandleError(Dart_GetType(Dart_RootLibrary(), Dart_NewStringFromCString("Object"), 0, nullptr));
    Dart_Handle dartProxy = HandleError(Dart_New(type, Dart_Null(), 1, args));

    return dartProxy;
  } else {
    Dart_ThrowException(Dart_NewApiError("unknown type"));
  }
}

jerry_value_t ResolveToJerry(Dart_Handle handle) {
  bool isUndefined = false;
  Dart_Handle undefined = HandleError(Dart_GetType(Dart_RootLibrary(), Dart_NewStringFromCString("Undefined"), 0, nullptr));
  HandleError(Dart_ObjectIsType(handle, undefined, &isUndefined));

  if (Dart_IsNull(handle)) {
    return jerry_create_null();
  } else if (isUndefined) {
    return jerry_create_undefined();
  } else if (Dart_IsBoolean(handle)) {
    bool value;
    HandleError(Dart_BooleanValue(handle, &value));

    return jerry_create_boolean(value);
  } else if (Dart_IsInteger(handle)) {
    int64_t integer;
    HandleError(Dart_IntegerToInt64(handle, &integer));

    return jerry_create_number(integer);
  } else if (Dart_IsDouble(handle)) {
    double integer;
    HandleError(Dart_DoubleValue(handle, &integer));

    return jerry_create_number(integer);
  } else if (Dart_IsString(handle)) {
    const char *value;
    HandleError(Dart_StringToCString(handle, &value));

    return jerry_create_string_from_utf8(reinterpret_cast<const jerry_char_t *>(value));
  } else if (Dart_IsTypedData(handle)) {
    Dart_TypedData_Type dartType;
    uint8_t *buf;
    intptr_t length;
    HandleError(Dart_TypedDataAcquireData(handle, &dartType, reinterpret_cast<void **>(&buf), &length));

    intptr_t arrayLength = length;

    jerry_typedarray_type_t type = JERRY_TYPEDARRAY_INVALID;
    if (dartType == Dart_TypedData_kInt8) {
      type = JERRY_TYPEDARRAY_INT8;
    } else if (dartType == Dart_TypedData_kUint8) {
      type = JERRY_TYPEDARRAY_UINT8;
    } else if (dartType == Dart_TypedData_kUint8Clamped) {
      type = JERRY_TYPEDARRAY_UINT8CLAMPED;
    } else if (dartType == Dart_TypedData_kInt16) {
      type = JERRY_TYPEDARRAY_INT16;
      length *= 2;
    } else if (dartType == Dart_TypedData_kUint16) {
      type = JERRY_TYPEDARRAY_UINT16;
      length *= 2;
    } else if (dartType == Dart_TypedData_kInt32) {
      type = JERRY_TYPEDARRAY_INT32;
      length *= 4;
    } else if (dartType == Dart_TypedData_kUint32) {
      type = JERRY_TYPEDARRAY_UINT32;
      length *= 4;
    } else if (dartType == Dart_TypedData_kFloat32) {
      type = JERRY_TYPEDARRAY_FLOAT32;
      length *= 4;
    } else if (dartType == Dart_TypedData_kFloat64) {
      type = JERRY_TYPEDARRAY_FLOAT64;
      length *= 8;
    }

    jerry_value_t arrayBuffer = jerry_create_arraybuffer(static_cast<jerry_length_t>(length));

    jerry_arraybuffer_write(arrayBuffer, 0, buf, static_cast<jerry_length_t>(length));

    jerry_value_t array = jerry_create_typedarray_for_arraybuffer_sz(type, arrayBuffer, 0, static_cast<jerry_length_t>(arrayLength));

    HandleError(Dart_TypedDataReleaseData(handle));

    return array;
  } else if (Dart_IsByteBuffer(handle)) {
    Dart_Handle typedData = Dart_GetDataFromByteBuffer(handle);

    Dart_TypedData_Type dartType;
    uint8_t *buf;
    intptr_t length;
    HandleError(Dart_TypedDataAcquireData(typedData, &dartType, reinterpret_cast<void **>(&buf), &length));

    if (dartType == Dart_TypedData_kInt16) {
      length *= 2;
    } else if (dartType == Dart_TypedData_kUint16) {
      length *= 2;
    } else if (dartType == Dart_TypedData_kInt32) {
      length *= 4;
    } else if (dartType == Dart_TypedData_kUint32) {
      length *= 4;
    } else if (dartType == Dart_TypedData_kFloat32) {
      length *= 4;
    } else if (dartType == Dart_TypedData_kFloat64) {
      length *= 8;
    }

    jerry_value_t arrayBuffer = jerry_create_arraybuffer(static_cast<jerry_length_t>(length));

    jerry_arraybuffer_write(arrayBuffer, 0, buf, static_cast<jerry_length_t>(length));

    HandleError(Dart_TypedDataReleaseData(typedData));

    return arrayBuffer;
  } else if (Dart_IsClosure(handle)) {
    jerry_value_t function = jerry_create_external_function([](const jerry_value_t func_value, const jerry_value_t this_value, const jerry_value_t *args_p, const jerry_length_t args_cnt) -> jerry_value_t {
      void *native_p;
      const jerry_object_native_info_t *type_p;
      bool has_p = jerry_get_object_native_pointer(func_value, &native_p, &type_p);

      Dart_PersistentHandle persistent = static_cast<Dart_PersistentHandle>(native_p);
      Dart_Handle handle = Dart_HandleFromPersistent(persistent);

      Dart_Handle *args = static_cast<Dart_Handle *>(malloc(args_cnt * sizeof(Dart_Handle *)));

      for (jerry_length_t i = 0; i < args_cnt; i++) {
        args[i] = ResolveToDart(args_p[i]);
      }

      return ResolveToJerry(HandleError(Dart_InvokeClosure(handle, args_cnt, args)));
    });

    Dart_PersistentHandle persistent = Dart_NewPersistentHandle(handle);
    Dart_SetPersistentHandle(persistent, handle);

    jerry_object_native_info_t *native_obj = static_cast<jerry_object_native_info_t *>(malloc(sizeof(jerry_object_native_info_t)));
    native_obj->free_cb = [](void *native_p) {
      Dart_PersistentHandle persistent = static_cast<Dart_PersistentHandle>(native_p);
      Dart_DeletePersistentHandle(persistent);
    };

    jerry_set_object_native_pointer(function, static_cast<void *>(persistent), native_obj);

    return function;
  } else if (Dart_IsList(handle)) {
    intptr_t length;
    HandleError(Dart_ListLength(handle, &length));

    jerry_value_t array = jerry_create_array(length);

    for (intptr_t i = 0; i < length; i++) {
      jerry_release_value(jerry_set_property_by_index(array, i, ResolveToJerry(HandleError(Dart_ListGetAt(handle, i)))));
    }

    return array;
  } else if (Dart_IsMap(handle)) {
    jerry_value_t object = jerry_create_object();

    Dart_Handle keys = HandleError(Dart_MapKeys(handle));

    intptr_t length;
    HandleError(Dart_ListLength(keys, &length));

    for (intptr_t i = 0; i < length; i++) {
      Dart_Handle key = HandleError(Dart_ListGetAt(keys, i));
      jerry_release_value(jerry_set_property(object, ResolveToJerry(key), ResolveToJerry(HandleError(Dart_MapGetAt(handle, key)))));
    }

    return object;
  } else if (Dart_IsError(handle)) {
    return jerry_create_error(JERRY_ERROR_COMMON, reinterpret_cast<const jerry_char_t *>(Dart_GetError(handle)));
  }
}

void ToString(Dart_NativeArguments arguments) {
  int64_t ptr;
  HandleError(Dart_GetNativeIntegerArgument(arguments, 0, &ptr));

  jerry_value_t string = jerry_value_to_string(reinterpret_cast<jerry_value_t>(static_cast<uint32_t>(ptr)));

  Dart_SetReturnValue(arguments, ResolveToDart(string));

  jerry_release_value(string);
}

void Get(Dart_NativeArguments arguments) {
  const char *key;
  HandleError(Dart_StringToCString(Dart_GetNativeArgument(arguments, 0), &key));

  int64_t ptr;
  HandleError(Dart_GetNativeIntegerArgument(arguments, 1, &ptr));

  jerry_value_t value = get(const_cast<char *>(key), ptr);

  Dart_SetReturnValue(arguments, ResolveToDart(value));
}

void Call(Dart_NativeArguments arguments) {
  int64_t ptr;
  HandleError(Dart_GetNativeIntegerArgument(arguments, 0, &ptr));

  int64_t thisArgp;
  HandleError(Dart_GetNativeIntegerArgument(arguments, 2, &thisArgp));

  bool doRet;
  HandleError(Dart_GetNativeBooleanArgument(arguments, 3, &doRet));

  jerry_value_t value = reinterpret_cast<jerry_value_t>(static_cast<uint32_t>(ptr));

  jerry_value_t thisArg;
  if (thisArgp > -1) {
    thisArg = reinterpret_cast<jerry_value_t>(static_cast<uint32_t>(thisArgp));
  } else {
    thisArg = jerry_create_undefined();
  }

  Dart_Handle argsList = Dart_GetNativeArgument(arguments, 1);

  intptr_t argsLength;
  HandleError(Dart_ListLength(argsList, &argsLength));

  jerry_value_t *args = static_cast<jerry_value_t *>(malloc(argsLength * sizeof(jerry_value_t)));

  for (intptr_t i = 0; i < argsLength; i++) {
    args[i] = ResolveToJerry(HandleError(Dart_ListGetAt(argsList, i)));
  }

  jerry_value_t ret = jerry_call_function(value, thisArg, args, argsLength);
  if (jerry_value_has_error_flag(ret)) {
    jerry_value_clear_error_flag(&ret);
    print_string_form(ret);
    jerry_release_value(ret);
    exit(1);
  }

  if (doRet) {
    Dart_SetReturnValue(arguments, ResolveToDart(ret));
  } else {
    Dart_SetReturnValue(arguments, Dart_Null());
  }

  jerry_release_value(ret);

  if (thisArgp < 0) {
    jerry_release_value(thisArg);
  }
}

void StartLoop(Dart_NativeArguments arguments) {
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void RunJerryJobs(Dart_NativeArguments arguments) {
  jerry_run_all_enqueued_jobs();
}

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope) {
  // If we fail, we return nullptr, and Dart throws an exception.
  if (!Dart_IsString(name)) return nullptr;
  Dart_NativeFunction result = nullptr;
  const char* cname;
  HandleError(Dart_StringToCString(name, &cname));

  if (strcmp("Init", cname) == 0) result = Init;
  if (strcmp("Cleanup", cname) == 0) result = Cleanup;
  if (strcmp("Load", cname) == 0) result = Load;
  if (strcmp("Get", cname) == 0) result = Get;
  if (strcmp("Call", cname) == 0) result = Call;
  if (strcmp("ToString", cname) == 0) result = ToString;
  if (strcmp("StartLoop", cname) == 0) result = StartLoop;
  if (strcmp("RunJerryJobs", cname) == 0) result = RunJerryJobs;
  return result;
}
