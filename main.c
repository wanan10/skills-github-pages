#include "display.h"

int main(int argc, char* argv[]) {
    // 初始化随机数种子
    srand((unsigned int)time(NULL));
    
    // 创建主控制器
    MainController* controller = displayCreate();
    if (!controller) {
        fprintf(stderr, "错误：无法初始化程序\n");
        return 1;
    }
    
    // 运行程序
    int result = displayRun(controller, argc, argv);
    
    // 清理资源
    displayDestroy(controller);
    
    return result;
}