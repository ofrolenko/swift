// RUN: %target-swift-ide-test -batch-code-completion -source-filename %s -filecheck %raw-FileCheck -completion-output-dir %t -enable-experimental-concurrency

// REQUIRES: concurrency

#^GLOBAL^#
// GLOBAL: Begin completions
// GLOBAL-DAG: Keyword/None:                       actor; name=actor
// GLOBAL-DAG: Keyword/None:                       await; name=await
// GLOBAL: End completion

enum Namespace {
    #^TYPEMEMBER^#
// TYPEMEMBER: Begin completions
// TYPEMEMBER-NOT: Keyword{{.*}}await
// TYPEMEMBER-DAG: Keyword/None:                       actor; name=actor
// TYPEMEMBER-NOT: Keyword{{.*}}await
// TYPEMEMBER: End completion
}

func testFunc() {
  #^STMT^#
// STMT: Begin completions
// STMT-DAG: Keyword/None/Flair[RareKeyword]:    actor; name=actor
// STMT-DAG: Keyword/None:                       await; name=await
// STMT: End completion
}

func testExpr() {
  _ = #^EXPR^#
// EXPR: Begin completions
// EXPR-NOT: Keyword{{.*}}actor
// EXPR-DAG: Keyword/None:                       await; name=await
// EXPR-NOT: Keyword{{.*}}actor
// EXPR: End completion
}
func testClosure() {
  func receiveClosure(_: () async -> Void) {}
  receiveClosure { #^IN_CLOSURE?check=STMT^# }
}

func testFunc(x: Int = #^FUNCDEFAULTPARAM?check=EXPR^#) {}

struct Something {
  var x = #^MEMBERINIT?check=EXPR^#
  func testFunc(x: Int = #^METHODDEFAULTPARAM?check=EXPR^#) {}
}
