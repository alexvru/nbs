#include <contrib/ydb/library/yql/public/udf/udf_value.h>
#include <contrib/ydb/library/yql/minikql/mkql_alloc.h>
#include <contrib/ydb/library/yql/minikql/mkql_terminator.h>

extern "C" void* UdfAllocate(ui64 size) {
    return ::NKikimr::NMiniKQL::MKQLAllocDeprecated(size, ::NKikimr::NMiniKQL::EMemorySubPool::Default);
}

extern "C" void UdfFree(const void* mem) {
    return ::NKikimr::NMiniKQL::MKQLFreeDeprecated(mem, ::NKikimr::NMiniKQL::EMemorySubPool::Default);
}

extern "C" [[noreturn]] void UdfTerminate(const char* message) {
    ::NKikimr::NMiniKQL::MKQLTerminate(message);
}

extern "C" void UdfRegisterObject(::NYql::NUdf::TBoxedValue*) {}

extern "C" void UdfUnregisterObject(::NYql::NUdf::TBoxedValue*) {}

extern "C" void* UdfAllocateWithSize(ui64 size) {
    return ::NKikimr::NMiniKQL::TWithDefaultMiniKQLAlloc::AllocWithSize(size);
}

extern "C" void UdfFreeWithSize(const void* mem, ui64 size) {
    return ::NKikimr::NMiniKQL::TWithDefaultMiniKQLAlloc::FreeWithSize(mem, size);
}
