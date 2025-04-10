/* 这个 #pragma once
 * 与 #ifndef/#define/#endif 是一样的效果
 */
#pragma once
#include <string>

namespace ErrorCode
{
    // 通用错误码
    const std::string CONTROLLER_OK = "EC-00-0000";                     // everything is ok(0)
    const std::string CONTROLLER_ERROR_TOKEN = "EC-00-0001";            // illegal token
    const std::string CONTROLLER_ERROR_ILLEGAL_ARGUMENT = "EC-00-0002"; // 错误的接口参数
    const std::string CONTROLLER_ERROR_FUNCTION_INVALID = "EC-00-0003"; // 接口方法不存在

    const std::string CONTROLLER_ERROR_DRIVER_ASSERT = "EC-00-0050";             // 断言错误
    const std::string CONTROLLER_ERROR_DRIVER_CONFIG = "EC-00-0051";             // 驱动配置文件异常
    const std::string CONTROLLER_ERROR_DRIVER_INIT = "EC-00-0052";               // 初始化异常
    const std::string CONTROLLER_ERROR_DRIVER_CONN = "EC-00-0053";               // 驱动连接异常
    const std::string CONTROLLER_ERROR_DRIVER_COMM = "EC-00-0054";               // 驱动通讯异常
    const std::string CONTROLLER_ERROR_DRIVER_ILLFUNC = "EC-00-0055";            // 方法异常
    const std::string CONTROLLER_ERROR_DRIVER_ILLPARAM = "EC-00-0056";           // 参数异常
    const std::string CONTROLLER_ERROR_DRIVER_EXECUTE = "EC-00-0057";            // 指令执行异常
    const std::string CONTROLLER_ERROR_DRIVER_MOTION_FAULT = "EC-00-0058";       // 电机运动时报错
    const std::string CONTROLLER_ERROR_DRIVER_MOTION_TIMEOUT = "EC-00-0059";     // 电机运动时超时
    const std::string CONTROLLER_ERROR_DRIVER_MOTION_LOST_TIMING = "EC-00-0060"; // 电机运动丢步
    const std::string CONTROLLER_ERROR_DRIVER_CONFLICT = "EC-00-0061";           // 运动状态冲突
    const std::string CONTROLLER_ERROR_DRIVER_COLLISION = "EC-00-0062";          // 机器人出现碰撞
    const std::string CONTROLLER_ERROR_DRIVER_ESTOP = "EC-00-0063";              // 机器人急停触发
    const std::string CONTROLLER_ERROR_DRIVER_FAULT = "EC-00-0064";              // 机器人关节发生错误

    const std::string CONTROLLER_ERROR_KINEMATIC_ASSERT = "EC-00-0100";      // 断言错误
    const std::string CONTROLLER_ERROR_KINEMATIC_ILLPARAM = "EC-00-0101";    // 解算参数错误
    const std::string CONTROLLER_ERROR_KINEMATIC_LOAD = "EC-00-0102";        // 解算模块加载错误
    const std::string CONTROLLER_ERROR_KINEMATIC_FORWARD = "EC-00-0103";     // 正解异常
    const std::string CONTROLLER_ERROR_KINEMATIC_INVERSE = "EC-00-0104";     // 逆解异常
    const std::string CONTROLLER_ERROR_KINEMATIC_NO_SOLUTION = "EC-00-0105"; // 无逆解结果
    const std::string CONTROLLER_ERROR_KINEMATIC_EXEC = "EC-00-0106";        // 执行方法异常

    const std::string CONTROLLER_ERROR_DYNAMICS_ASSERT = "EC-00-0120";   // 断言错误
    const std::string CONTROLLER_ERROR_DYNAMICS_ILLPARAM = "EC-00-0121"; // 动力学参数错误
    const std::string CONTROLLER_ERROR_DYNAMICS_LOAD = "EC-00-0122";     // 动力学模块加载错误
    const std::string CONTROLLER_ERROR_DYNAMICS_EXEC = "EC-00-0123";     // 执行方法异常

    const std::string CONTROLLER_ERROR_EOAT_ASSERT = "EC-00-0150";   // 断言错误
    const std::string CONTROLLER_ERROR_EOAT_ILLPARAM = "EC-00-0151"; // 末端工具参数错误
    const std::string CONTROLLER_ERROR_EOAT_LOAD = "EC-00-0152";     // 末端工具加载错误
    const std::string CONTROLLER_ERROR_EOAT_EXEC = "EC-00-0153";     // 执行方法异常
    const std::string CONTROLLER_ERROR_EOAT_CONFIG = "EC-00-0154";   // 末端工具文件异常

    const std::string CONTROLLER_ERROR_TRAJECTORY_ASSERT = "EC-00-0250";   // 断言错误
    const std::string CONTROLLER_ERROR_TRAJECTORY_ILLPARAM = "EC-00-0251"; // 轨迹参数错误
    const std::string CONTROLLER_ERROR_TRAJECTORY_LOAD = "EC-00-0252";     // 轨迹模块加载错误
    const std::string CONTROLLER_ERROR_TRAJECTORY_EXEC = "EC-00-0254";     // 执行方法异常

    const std::string CONTROLLER_ERROR_COLLISION_ASSERT = "EC-00-0300";   // 碰撞模块断言错误
    const std::string CONTROLLER_ERROR_COLLISION_ILLPARAM = "EC-00-0301"; // 碰撞模块参数错误
    const std::string CONTROLLER_ERROR_COLLISION_LOAD = "EC-00-0302";     // 碰撞模块加载错误
    const std::string CONTROLLER_ERROR_COLLISION_EXEC = "EC-00-0304";     // 碰撞模块执行方法异常

    const std::string CONTROLLER_ERROR_TEACHING_ASSERT = "EC-00-0350";   // 示教模块断言错误
    const std::string CONTROLLER_ERROR_TEACHING_ILLPARAM = "EC-00-0351"; // 示教模块参数错误
    const std::string CONTROLLER_ERROR_TEACHING_LOAD = "EC-00-0352";     // 示教模块加载错误
    const std::string CONTROLLER_ERROR_TEACHING_EXEC = "EC-00-0354";     // 示教模块执行方法异常

    const std::string CONTROLLER_ERROR_FROZEN = "EC-00-1000";  // 无法执行异常
    const std::string CONTROLLER_ERROR_EXECUTE = "EC-00-1001"; // 执行异常
    const std::string CONTROLLER_ERROR_INIT = "EC-00-1002";    // 初始化异常
    const std::string CONTROLLER_ERROR_NO_INIT = "EC-00-1003"; // 未初始化
    const std::string CONTROLLER_ERROR_PARAMS = "EC-00-1004";  // 参数异常
    const std::string CONTROLLER_ERROR_ESTOP = "EC-00-1005";   // 机械臂急停
    const std::string CONTROLLER_ERROR_TYPE = "EC-00-1006";    // 类型错误
    const std::string CONTROLLER_ERROR_ASSERT = "EC-00-1007";  // 断言错误
    const std::string CONTROLLER_ERROR_CONFIG = "EC-00-1008";  // 配置文件异常
    const std::string CONTROLLER_ERROR_CALIB = "EC-00-1009";   // 未校准
    const std::string CONTROLLER_ERROR_LOAD = "EC-00-1010";    // 未加载
    const std::string CONTROLLER_ERROR_UNKNOWN = "EC-00-9999"; // 未知错误

    const std::string CONTROLLER_ERROR_UNIT_ADD = "EC-00-2000";  // 加数异常
    const std::string CONTROLLER_ERROR_UNIT_RADD = "EC-00-2001"; // 被加数异常
    const std::string CONTROLLER_ERROR_UNIT_SUB = "EC-00-2002";  // 减数异常
    const std::string CONTROLLER_ERROR_UNIT_RSUB = "EC-00-2003"; // 被减数异常
    const std::string CONTROLLER_ERROR_UNIT_MUL = "EC-00-2004";  // 乘法异常
    const std::string CONTROLLER_ERROR_UNIT_RMUL = "EC-00-2005"; // 被乘法异常
    const std::string CONTROLLER_ERROR_UNIT_FMT = "EC-00-2006";  // 格式异常
    const std::string CONTROLLER_ERROR_UNIT_UM = "EC-00-2007";   // 模式异常
    const std::string CONTROLLER_ERROR_UNIT_DIV = "EC-00-2008";  // 被除法异常
    const std::string CONTROLLER_ERROR_UNIT_POW = "EC-00-2009";  // 乘方异常
}
