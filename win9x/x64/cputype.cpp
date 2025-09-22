#include "cputype.h"

// x64アーキテクチャでは常にMMXがサポートされている
int havemmx(void) {
    return 1; // 常にtrue（MMXサポート）を返す
}