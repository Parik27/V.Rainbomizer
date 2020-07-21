#include "rage.hh"

/*******************************************************/
uint32_t rage::atStringHashLowercase (const char *param_1)
{
    int8_t   cVar1;
    int8_t   cVar2;
    uint32_t uVar3;

    cVar1 = *param_1;
    uVar3 = 0;
    if (cVar1 == '\"')
        {
            param_1 = param_1 + 1;
        }
    cVar2 = *param_1;
    while ((cVar2 != '\0' && ((cVar1 != '\"' || (cVar2 != '\"')))))
        {
            param_1 = param_1 + 1;
            if ((uint8_t) (cVar2 + 0xbfU) < 0x1a)
                {
                    cVar2 = cVar2 + ' ';
                }
            else
                {
                    if (cVar2 == '\\')
                        {
                            cVar2 = '/';
                        }
                }
            uVar3 = ((uint32_t) cVar2 + uVar3) * 0x401;
            uVar3 = uVar3 ^ uVar3 >> 6;
            cVar2 = *param_1;
        }
    uVar3 = (uVar3 * 9 >> 0xb ^ uVar3 * 9) * 0x8001;
    if (uVar3 < 2)
        {
            uVar3 = uVar3 + 2;
        }
    return uVar3;
}
