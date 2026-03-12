#ifndef USER_MODE_H
#define USER_MODE_H

/* Hardware context switch out of the kernel into Ring 3 */
void switch_to_user_mode(void);

#endif /* USER_MODE_H */
