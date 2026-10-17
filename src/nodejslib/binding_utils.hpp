#pragma once
#include <napi.h>
#include <memory>
#include <stdexcept>

// node-addon-api translates Napi::Error, but does not catch std::exception.
template<class Function>
auto nativeCall(Napi::Env env, Function function) -> decltype(function()) {
    try { return function(); }
    catch (const Napi::Error&) { throw; }
    catch (const std::invalid_argument& error) { throw Napi::TypeError::New(env, error.what()); }
    catch (const std::exception& error) { throw Napi::Error::New(env, error.what()); }
}
