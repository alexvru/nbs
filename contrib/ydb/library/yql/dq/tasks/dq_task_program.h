#pragma once

#include <contrib/ydb/library/yql/dq/expr_nodes/dq_expr_nodes.h>
#include <contrib/ydb/library/yql/dq/tasks/dq_tasks_graph.h>

#include <contrib/ydb/library/yql/minikql/mkql_node.h>
#include <contrib/ydb/library/yql/providers/common/mkql/yql_provider_mkql.h>
#include <contrib/ydb/library/yql/providers/common/provider/yql_provider.h>

namespace NYql::NDq {

const TStructExprType* CollectParameters(NNodes::TCoLambda program, TExprContext& ctx);

TString BuildProgram(NNodes::TCoLambda program, const TStructExprType& paramsType,
                     const NCommon::IMkqlCallableCompiler& compiler, const NKikimr::NMiniKQL::TTypeEnvironment& typeEnv,
                     const NKikimr::NMiniKQL::IFunctionRegistry& funcRegistry, TExprContext& exprCtx,
                     const TVector<NNodes::TExprBase>& reads);

} // namespace NYql::NDq
