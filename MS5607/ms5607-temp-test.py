#!/usr/bin/env python3
from matplotlib import pyplot as plt
import numpy as np

def set_in_range(nums, lower_bound, upper_bound):
    print('flag')
    for i in range(len(nums)):
        if (nums[i] < lower_bound):
            nums[i] = lower_bound
            print('too low')
        elif (nums[i] > upper_bound):
            nums[i] = upper_bound
            print('too high')
    return
        

def main():

    # Conversion constants
    C1 = np.uint16(42836)  # uint16_t from SPI
    C2 = np.uint16(38938)
    C3 = np.uint16(26130)
    C4 = np.uint16(23835)
    C5 = np.uint16(32083)
    C6 = np.uint16(27666)
    
    # Computing temps
    D1 = np.uint32(6225888)    # uint32_t from SPI
    
    D2 = np.arange(0, 2**24-1, 100, dtype='uint32')  # uint32_t from SPI
    #D2 = np.arange(6000000, 11000000, 100, dtype='uint32')  # uint32_t from SPI
    N = len(D2)
    
    dT = np.int64(D2 - np.uint64(C5)*pow(2, 8))   #always cast dT as >= int64 or >= float32. python needs C5 cast as >= uint64 but C code doesn't have this issue
    #set_in_range(dT, -16776960, 16777216)   # no effect

    TEMP = np.int32(2000 + dT*C6/pow(2, 23))    # fine
    
    OFF =  np.int64(C2*pow(2, 17) + (C4*dT)/pow(2, 6)) # fine
    #set_in_range(OFF, -17179344900, 25769410560)    # no effect
    
    SENS = np.int64(C1*pow(2, 16) +(C3*dT)/pow(2, 7))  # fine
    
    #print('dT: %d %d' % (dT.min(), dT.max()))
    #print('OFF: %d %d' % (OFF.min(), OFF.max()))
    #print('SENS: %d %d' % (SENS.min(), SENS.max()))
    
    T2 = np.zeros(N, dtype='int64')
    OFF2 = np.zeros(N, dtype='int64')
    SENS2 = np.zeros(N, dtype='int64')
    
    # Temperature Compensation
    for i in range(N):
        if TEMP[i] < 2000:
            T2[i] = np.int64(pow(dT[i], 2)/pow(2, 31))
            OFF2[i] = np.int64(61*np.int64(pow(TEMP[i] - 2000, 2))/pow(2, 4))
            SENS2[i] = np.int64(2*pow(TEMP[i] - 2000, 2))
            
            if TEMP[i] < -1500:
                OFF2[i] = np.int64(OFF2[i] + 15 * np.int64(pow(TEMP[i] + 1500, 2)))
                SENS2[i] = np.int64(SENS2[i] + 8 * np.int64(pow(TEMP[i] + 1500, 2)))
            
    TEMP = TEMP - T2
    OFF = OFF - OFF2
    SENS = SENS - SENS2
    $set_in_range(SENS, -8589672450, 12884705280)
    
    PRES = np.int32((D1*SENS/pow(2, 21) - OFF)/pow(2,15))
    
    #print('dT_corr: %d %d' % (dT.min(), dT.max()))
    #print('OFF_corr: %d %d' % (OFF.min(), OFF.max()))
    #print('SENS_corr: %d %d' % (SENS.min(), SENS.max()))

    # Plotting
    fig, axs = plt.subplots(5, 1, sharex=True)
    axs[0].plot(D2, dT)
    axs[0].set_ylabel('dT')
    axs[1].plot(D2, OFF)
    axs[1].set_ylabel('OFF')
    axs[2].plot(D2, SENS)
    axs[2].set_ylabel('SENS')
    axs[3].plot(D2, TEMP)
    axs[3].plot(D2, np.full(N, -4000))
    axs[3].plot(D2, np.full(N, -1500))
    axs[3].plot(D2, np.full(N, 2000))
    axs[3].plot(D2, np.full(N, 8500))
    axs[4].plot(D2, PRES)
    axs[4].set_ylabel('PRES')
    #axs[3].legend()
    axs[3].set_ylabel('TEMP')
    
    plt.tight_layout()
    plt.savefig('ms5607-correction-test.png')
    plt.show()
    
    '''
    fig, ax = plt.subplots(2, 1, sharex=True)
    ax[0].set_title('TOP TEXT')
    #ax[0].plot(D2, dT, label='dT')
    ax[0].plot(D2, T2, label='T2')
    ax[0].legend()
    ax[0].set_ylabel('dT')
    ax[1].plot(D2, TEMP)
    ax[1].set_ylabel('TEMP')
    '''
    
    '''
    fig, ax = plt.subplots(1,1,sharex=True)
    ax[0].set_title('TOP TEXT')
    ax[0].plot(D2, dT)
    ax[0].plot(D2, dT2)
    ax[0].set_ylabel('dT [%s]' % (str(dT.dtype)))
    '''
    
    '''
    plt.tight_layout()
    plt.savefig('ms5607-temp-test.png')
    plt.show()
    '''
    
    return
    
    
if __name__ == '__main__':
    main()