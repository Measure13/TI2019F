#include "USART_HMI.h"
#include <string.h>
#include <math.h>
//! give a reset signal every time a program starts!

volatile bool initialization_done = false;
volatile bool ready_to_receive = false;
volatile bool receive_done = false;
static uint8_t data_tmp_write[MAX_SEND_DATA];

void UARTHMI_Forget_It(void)
{
    printf("\x00\xff\xff\xff");
}

static int UARTHMI_Append_Ending(uint8_t *dest)
{
    const int ending_len = 3;
    const uint8_t ending[3] = {0xFF, 0xFF, 0xFF};
    int dest_end_index = strlen((char *)dest);

    for (int i = 0; i < ending_len; i++)
    {
        dest[dest_end_index++] = ending[i];
    }
    return dest_end_index;
}

/// @brief Draw a curve on the UART HMI
/// @param index the **index**th plotter to use
/// @param pf the pointer to the data head
/// @param num number of data
/// @param margin the width of the margin between the curve and the upper and lower boundaries of the screen. If you don't care, just give 0
void UARTHMI_Draw_Curve_addt(int index, float *pf, uint16_t num, uint8_t margin)
{
    int i, total_num, send_num, interval_num;
    float max_data = pf[0], coef = 0, min_data = pf[0];

    total_num = num;
    interval_num = num / MAX_SEND_LEN; // adjust here every time reuse it
    if (interval_num)
        send_num = num / interval_num;
    else
    {
        interval_num = 1;
        send_num = num;
    }
    printf("addt s%d.id,0,%d\xff\xff\xff", index, send_num);
    while (!ready_to_receive)
    {
    }
    ready_to_receive = false;
    for (i = total_num - 1; i >= 0; i -= interval_num)
    {
        if (max_data < pf[i])
        {
            max_data = pf[i];
        }
        if (min_data > pf[i])
        {
            min_data = pf[i];
        }
    }
    coef = (MAX_SEND_DATA - 2 * margin) / (max_data - min_data);
    for (i = total_num - 1; i >= 0; i -= interval_num)
    {
        data_tmp_write[(total_num - 1 - i) / interval_num] = (uint8_t)((pf[i] - min_data) * coef + margin);
    }
    USART_Send_Data_Direct(data_tmp_write, send_num);
    while (!receive_done)
    {
    }
    receive_done = false;
}

static uint8_t UARTHMI_Get_Integer_Digits(int integer)
{
    int temp;
    uint8_t cnt = 0;
    if (integer > 0)
    {
        temp = integer; // this should be optimized. temp is not needed.
        while (temp)
        {
            temp /= 10;
            ++cnt;
        }
        return cnt;
    }
    else if (integer == 0)
    {
        return 1;
    }
    else
    {
        integer = -integer;
        while (integer)
        {
            integer /= 10;
            ++cnt;
        }
        return cnt + 1;
    }
}

//static int UARTHMI_Float_Adjust(float float_num, uint8_t digits_for_integer, uint8_t digits_for_decimals)
//{
//    // ESP_LOGW("fad", "distortion:%d", (int)float_num);
//    uint8_t integer_len = UARTHMI_Get_Integer_Digits((int)float_num);
//    // ESP_LOGW("fad", "distortion len:%u", integer_len);
//    int adjusted_float = 0;
//    integer_len = digits_for_integer + digits_for_decimals - integer_len;
//    float_num *= powf(10.0f, integer_len);
//    adjusted_float = (int)float_num;
//    return adjusted_float;
//}

//static void UARTHMI_Set_Float(int index, float float_num, uint8_t digits_for_integer, uint8_t digits_for_decimals)
//{
//    uint8_t data_len;
//    uint8_t *send_str;
//    uint8_t len = UARTHMI_Get_Integer_Digits(index) + 11 + digits_for_integer + digits_for_decimals;
//    send_str = (uint8_t *)malloc(sizeof(uint8_t) * (len));
//    // if (!send_str)ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
//    memset(send_str, 0, sizeof(uint8_t) * (len));
//    // ESP_LOGE(TAG, "distortion:%f", float_num);
//    sprintf((char *)send_str, "x%d.val=%d", index, UARTHMI_Float_Adjust(float_num, digits_for_integer, digits_for_decimals));
//    data_len = UARTHMI_Append_Ending(send_str);
//    USART_Send_Data_Direct(send_str, data_len);
//    // ESP_LOGI(TAG, "write done, size:%d", data_len);
//    free(send_str);
//}

//void UARTHMI_Send_Float(int index, float float_num)
//{
//    //UARTHMI_Set_Float(index, float_num, 1, 2);
//    printf("x%d.val=%d\xff\xff\xff", index, (int)(float_num * 100));
//}

void UARTHMI_Set_Text(uint8_t index, uint8_t *char_p)
{
    uint8_t send_len;
    uint8_t len = UARTHMI_Get_Integer_Digits(index) + strlen((char *)char_p) + 11 + 5; // +5 for safe
    uint8_t *send_str = (uint8_t *)malloc(len * sizeof(uint8_t));
    memset(send_str, 0, sizeof(uint8_t) * (len));
    sprintf((char *)send_str, "t%d.txt=\"%s\"", index, char_p);
    send_len = UARTHMI_Append_Ending(send_str);
    USART_Send_Data_Direct(send_str, send_len);
    free(send_str);
}


void UARTHMI_Set_Number(uint8_t index, int number)
{
    uint8_t send_len;
    uint8_t len = UARTHMI_Get_Integer_Digits(index) + UARTHMI_Get_Integer_Digits(number) + 11 + 5; // +5 for safe
    uint8_t *send_str = (uint8_t *)malloc(len * sizeof(uint8_t));
    memset(send_str, 0, sizeof(uint8_t) * (len));
    sprintf((char *)send_str, "n%d.val=%d", index, number);
    send_len = UARTHMI_Append_Ending(send_str);
    USART_Send_Data_Direct(send_str, send_len);
    free(send_str);
}

void UARTHMI_Send_Number(uint8_t index, int number)
{
    UARTHMI_Set_Number(index, number);
}

void UARTHMI_Reset(void)
{
    printf("rest\xff\xff\xff");
}

void UARTHMI_Visibility_Change(uint8_t txt_index, bool visiblity)
{
    printf("vis t%d,%d\xff\xff\xff", txt_index, visiblity);
}

void UARTHMI_Cross_Page_Set_Number(uint8_t index, int number, char* page_name)
{
    char mes[30];
    memset(mes, 0x00, sizeof(char) * 30);
    sprintf(mes, "%s.n%d.val=%d\xff\xff\xff", page_name, index, number);
    USART_Send_Data_Direct(mes, strlen((const char*)mes));
}
