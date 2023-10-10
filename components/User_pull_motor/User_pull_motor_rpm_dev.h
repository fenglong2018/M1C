#ifndef _USER_PULL_MOTOR_RPM_DEV_H_
#define _USER_PULL_MOTOR_RPM_DEV_H_

#define GPIO_RPM_CAP_IN   (15)   //Set GPIO 15 as  CAP0



/*初始化RPM检测*/
void user_pull_motor_rpm_dev_init(void);

/*调用后，如果rpm线程存在，那么会对线程进行删除退出*/
void rpm_set_close(void);

#endif
