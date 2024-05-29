/*
 * Copyright (c) 2024 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "common_api.h"
#include "pwrkey.h"

/*
 * demo演示pwrkey长按开机操作
 * 加入下面的代码即可
 * 源码文件需要放到bootloader目录下，名称可以随意，代码是编译进bootloader，和客户项目代码无关
 * 一旦放到bootloader目录下，所有项目编译后都会有效果
 */
void user_code_init(void)
{
	//if(pwrKeyGetPwrKeyMode() == PWRKEY_PWRON_MODE)	//这个打开的话，就需要luat_pm_power_ctrl(LUAT_PM_POWER_POWERKEY_MODE, 1)才能有效
	pwrkeyPwrOnDebounce(2000); //长按2000ms开机，时间自己控制，如果不写，则pwrkey短按就开机
}
