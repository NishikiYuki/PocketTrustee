#include "ll_multiplexer.hpp"
#include "main.h"

MultiplexerLowLayer& MultiplexerLowLayer::getInstance() {
    static MultiplexerLowLayer instance;
    return instance;
}

void MultiplexerLowLayer::ll_init() {
    HF_EN1_GPIO_Port->BSRR = HF_EN1_Pin;
}

void MultiplexerLowLayer::set_channel(MultiplexerChannel channel) {
    HF_EN1_GPIO_Port->BSRR = HF_EN1_Pin;
    if (channel == MUX_NONE) return;
    if (channel == MUX_BYPASS)
    {
        HF_S3_GPIO_Port->BSRR = HF_S3_Pin;
    }
    else
    {
        HF_EN1_GPIO_Port->BRR = HF_EN1_Pin;
        switch (channel)
        {
        case MUX_CHANNEL1:
            // Card1 -> L L
            HF_S1_GPIO_Port->BRR = HF_S1_Pin;
            HF_S2_GPIO_Port->BRR = HF_S2_Pin;
            break;
        case MUX_CHANNEL2:
            // Card2 -> H L
            HF_S1_GPIO_Port->BSRR = HF_S1_Pin;
            HF_S2_GPIO_Port->BRR = HF_S2_Pin;
            break;
        case MUX_CHANNEL3:
            // Card3 -> L H
            HF_S1_GPIO_Port->BRR = HF_S1_Pin;
            HF_S2_GPIO_Port->BSRR = HF_S2_Pin;
            break;
        case MUX_CHANNEL4:
            // Card4 -> H H
            HF_S1_GPIO_Port->BSRR = HF_S1_Pin;
            HF_S2_GPIO_Port->BSRR = HF_S2_Pin;
        default:
            break;
        }
    }
}
