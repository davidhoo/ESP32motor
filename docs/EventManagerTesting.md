# EventManager 测试指南

## 概述
本文档介绍了如何使用EventManager的测试功能，包括测试覆盖范围、运行方法和结果解读。

## 测试覆盖范围

EventManager测试覆盖了以下功能：

1. **单例模式测试** - 验证EventManager单例模式的正确性
2. **初始化和清理测试** - 测试初始化和资源清理功能
3. **事件订阅和取消订阅测试** - 验证事件监听器的注册和注销
4. **同步事件发布测试** - 测试立即执行的事件发布
5. **异步事件发布测试** - 测试加入队列的异步事件发布
6. **事件队列处理测试** - 验证事件队列的处理机制
7. **多监听器处理测试** - 测试一个事件类型对应多个监听器的情况
8. **事件类型名称获取测试** - 验证事件类型到字符串的转换
9. **错误处理测试** - 测试各种错误情况的处理
10. **边界条件测试** - 测试大量事件和边界值的处理

## 如何运行测试

### 方法1：使用测试主程序
1. 修改 `src/test_main.cpp` 中的 `currentTestMode`：
   ```cpp
   TestMode currentTestMode = EVENT_MANAGER_TEST_MODE;
   ```

2. 编译并上传程序到ESP32
3. 打开串口监视器，波特率115200
4. 观察测试输出结果

### 方法2：独立运行测试
在代码中直接调用：
```cpp
#include "tests/EventManagerTest.h"

// 运行所有测试
EventManagerTest::runAllTests();
```

## 测试输出示例

```
=== EventManager 单元测试开始 ===
测试单例模式...
✓ 单例模式应该返回相同实例
✓ 单例模式测试通过
测试初始化和清理...
✓ 初始化应该成功
✓ 重复初始化应该返回true
✓ 清理后重新初始化应该成功
✓ 初始化和清理测试通过
...
=== EventManager 单元测试完成 ===
```

## 测试文件结构

- `src/tests/EventManagerTest.h` - 测试类头文件
- `src/tests/EventManagerTest.cpp` - 测试类实现
- `src/test_main.cpp` - 测试主程序（已集成EventManager测试）

## 添加新测试

要添加新的测试用例：

1. 在 `EventManagerTest.h` 中添加测试方法声明
2. 在 `EventManagerTest.cpp` 中实现测试方法
3. 在 `runAllTests()` 方法中调用新测试
4. 遵循现有的测试模式和断言风格

## 注意事项

- 测试使用串口输出结果，确保串口连接正常
- 测试会创建和销毁EventManager实例，不要在生产环境中运行
- 所有测试都是独立的，可以单独运行
- 测试使用静态变量跟踪状态，运行前会自动重置

## 故障排除

如果测试失败：

1. 检查串口输出中的错误信息
2. 确认EventManager已正确初始化
3. 检查是否有内存不足的情况
4. 验证事件监听器是否正确注册

## 集成到CI/CD

可以将EventManager测试集成到持续集成流程中：

```bash
# 编译测试版本
pio run -e test

# 运行测试（需要连接ESP32）
pio run -e test -t upload
```

测试通过标准：所有断言都显示"✓"标记，无"❌"失败标记。