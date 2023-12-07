-----------

#include <linux/kernel.h>     // kstrtouint
#include <linux/module.h>     // MODULE_ macros
#include <linux/init.h>       // __init
#include <linux/kobject.h>    // kobject, kobject_atribute,
                              // kobject_create_and_add, kobject_put
#include <asm/io.h>           // iowrite, ioread, ioremap_nocache (platform specific)
#include "../address_map.h"   // overall memory map
#include "wave_regs.h"          // register offsets in QE IP

//-----------------------------------------------------------------------------
// Kernel module information
//-----------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Edgar Hernandez");
MODULE_DESCRIPTION("Wave IP Driver");

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

static unsigned int *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void runChannel(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_RUN);
    iowrite32(value | (1 << channel), base + OFS_RUN);
}

void stopChannel(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_RUN);
    iowrite32(value & ~(1 << channel), base + OFS_RUN);
}

unsigned int isChannelRunning(void)
{
    return (ioread32(base + OFS_RUN) & 0x3);
}

void setMode(uint8_t channel, uint8_t mode)
{
	unsigned int value = ioread32(base + OFS_MODE); 
	uint8_t clearMask = ~(7 << (3 * channel));
	uint8_t setMask = (mode << (3 * channel));
	value &= clearMask;
	iowrite32(value, base + OFS_MODE); 
	value |= setMask;
    iowrite32(value, base + OFS_MODE);
}

int32_t getMode(uint8_t channel)
{
    if(channel == 1)
	{
		return ioread32(base + OFS_MODE) >> 3;
	}
	else
	{
		return ioread32(base + OFS_MODE) & 0x00000007;
	}		
}

void setFreq(uint8_t channel, uint32_t freq)
{
    iowrite32(freq, (base + OFS_FREQ_A + channel));
}

uint32_t getFreq(uint8_t channel)
{
    return ioread32(base + OFS_FREQ_A + channel);
}

void setOfs(uint8_t channel, int16_t offset)
{
    int value = ioread32(base + OFS_OFFSET);
	uint32_t clearMask = ~(0x0000FFFF << (16 * channel));
	uint32_t setMask = (offset << (16 * channel));
	value &= clearMask;
	iowrite32(value, base + OFS_OFFSET);
	value |= setMask;
	iowrite32(value, base + OFS_OFFSET);
}

int getOfs(uint8_t channel)
{
	int value;
    if(channel == 1)
	{
		value = ioread32(base + OFS_OFFSET) >> 16;
		if(value >= 0x8000)
		{
			value |= 0xFFFF0000;
		}
		return value;
	}
	else
	{
		value = ioread32(base + OFS_OFFSET) & 0x0000FFFF;
		if(value >= 0x8000)
		{
			value |= 0xFFFF0000;
		}
		return value;
	}
}

void setAmplitude(uint8_t channel, uint16_t amplitude)
{
    unsigned int value = ioread32(base + OFS_AMPLITUDE);
	uint32_t clearMask = ~(0x0000FFFF << (16 * channel));
	uint32_t setMask = (amplitude << (16 * channel));
	value &= clearMask;
	iowrite32(value, base + OFS_AMPLITUDE);
	value |= setMask;
	iowrite32(value, base + OFS_AMPLITUDE);
}

uint16_t getAmplitude(uint8_t channel)
{
    if(channel == 1)
	{
		return ioread32(base + OFS_AMPLITUDE) >> 16;
	}
	else
	{
		return ioread32(base + OFS_AMPLITUDE) & 0x0000FFFF;
	}
}

void setDty(uint8_t channel, uint16_t duty)
{
    unsigned int value = ioread32(base + OFS_DTYCYC);
	uint32_t clearMask = ~(0x0000FFFF << (16 * channel));
	uint32_t setMask = (duty << (16 * channel));
	value &= clearMask;
	iowrite32(value, base + OFS_DTYCYC);
	value |= setMask;
	iowrite32(value, base + OFS_DTYCYC);
}

uint16_t getDty(uint8_t channel)
{
    if(channel == 1)
	{
		return ioread32(base + OFS_DTYCYC) >> 16;
	}
	else
	{
		return ioread32(base + OFS_DTYCYC) & 0x0000FFFF;
	}
}

void setCycles(uint8_t channel, uint16_t cycles)
{
    unsigned int value = ioread32(base + OFS_CYCLES);
	uint32_t clearMask = ~(0x0000FFFF << (16 * channel));
	uint32_t setMask = (cycles << (16 * channel));
	value &= clearMask;
	iowrite32(value, base + OFS_CYCLES);
	value |= setMask;
	iowrite32(value, base + OFS_CYCLES);
}

uint16_t getCycles(uint8_t channel)
{
    if(channel == 1)
	{
		return ioread32(base + OFS_CYCLES) >> 16;
	}
	else
	{
		return ioread32(base + OFS_CYCLES) & 0x0000FFFF;
	}
}

//-----------------------------------------------------------------------------
// Kernel Objects
//-----------------------------------------------------------------------------

// Enable 0

// Position 0
static int mode0 = 0;
module_param(mode0, int, S_IRUGO);
MODULE_PARM_DESC(mode0, " Mode of channel A");

static ssize_t mode0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{	
	if (strncmp(buffer, "dc", count-1) == 0)
    {
        setMode(0, 0);
    }
    else if (strncmp(buffer, "sine", count-1) == 0)
    {
        setMode(0, 1);
    }
    else if (strncmp(buffer, "sawtooth", count-1) == 0)
    {
        setMode(0, 2);
    }
	else if (strncmp(buffer, "triangle", count-1) == 0)
    {
        setMode(0, 3);
    }
	else if (strncmp(buffer, "square", count-1) == 0)
    {
        setMode(0, 4);
    }
    return count;
}

static ssize_t mode0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    mode0 = getMode(0);
	
    if(mode0 == 0)
	{
        strcpy(buffer, "DC\n");
	}
    else if(mode0 == 1)
	{
        strcpy(buffer, "Sine\n");
	}
    else if(mode0 == 2)
	{
        strcpy(buffer, "Sawtooth\n");
	}
    else if(mode0 == 3)
	{
        strcpy(buffer, "Triangle\n");
	}
    else if(mode0 == 4)
	{
        strcpy(buffer, "Square\n");
	}
    else if(mode0 == 5)
	{
        strcpy(buffer, "Arb\n");
	}
    return strlen(buffer);
}

static struct kobj_attribute mode0Attr = __ATTR(mode0, 0664, mode0Show, mode0Store);

static int freq0 = 0;
module_param(freq0, int, S_IRUGO);
MODULE_PARM_DESC(freq0, " Frequency of channel A");

static ssize_t freq0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &freq0);
    if (result == 0)
        setFreq(0, freq0);
    return count;
}

static ssize_t freq0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    freq0 = getFreq(0);
    return sprintf(buffer, "%d\n", freq0);
}

static struct kobj_attribute freq0Attr = __ATTR(freq0, 0664, freq0Show, freq0Store);

static int amplitude0 = 0;
module_param(amplitude0, int, S_IRUGO);
MODULE_PARM_DESC(amplitude0, " Amplitude of channel A");

static ssize_t amplitude0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &amplitude0);
    if (result == 0)
        setAmplitude(0, amplitude0);
    return count;
}

static ssize_t amplitude0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    amplitude0 = getAmplitude(0);
    return sprintf(buffer, "%d\n", amplitude0);
}

static struct kobj_attribute amplitude0Attr = __ATTR(amplitude0, 0664, amplitude0Show, amplitude0Store);


static int offset0 = 0;
module_param(offset0, int, S_IRUGO);
MODULE_PARM_DESC(offset0, " Offset of channel A");

static ssize_t offset0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtoint(buffer, 0, &offset0);
    if (result == 0)
        setOfs(0, offset0);
    return count;
}

static ssize_t offset0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    offset0 = getOfs(0);
    return sprintf(buffer, "%d\n", offset0);
}

static struct kobj_attribute offset0Attr = __ATTR(offset0, 0664, offset0Show, offset0Store);

static int dty0 = 0;
module_param(dty0, int, S_IRUGO);
MODULE_PARM_DESC(dty0, " Duty cycle of channel A");

static ssize_t dty0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &dty0);
    if (result == 0)
        setDty(0, dty0);
    return count;
}

static ssize_t dty0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    dty0 = getDty(0);
    return sprintf(buffer, "%d\n", dty0);
}

static struct kobj_attribute dty0Attr = __ATTR(dty0, 0664, dty0Show, dty0Store);

static int cycles0 = 0;
module_param(cycles0, int, S_IRUGO);
MODULE_PARM_DESC(cycles0, " Cycles of channel A");

static ssize_t cycles0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &cycles0);
    if (result == 0)
        setCycles(0, cycles0);
    return count;
}

static ssize_t cycles0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    dty0 = getCycles(0);
    return sprintf(buffer, "%d\n", cycles0);
}

static struct kobj_attribute cycles0Attr = __ATTR(cycles0, 0664, cycles0Show, cycles0Store);

// Channel B ------------------------------------------------------------------------------------------------------

static int mode1 = 0;
module_param(mode1, int, S_IRUGO);
MODULE_PARM_DESC(mode1, " Mode of channel B");

static ssize_t mode1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	if (strncmp(buffer, "dc", count-1) == 0)
    {
        setMode(1, 0);
    }
    else if (strncmp(buffer, "sine", count-1) == 0)
    {
        setMode(1, 1);
    }
    else if (strncmp(buffer, "sawtooth", count-1) == 0)
    {
        setMode(1, 2);
    }
	else if (strncmp(buffer, "triangle", count-1) == 0)
    {
        setMode(1, 3);
    }
	else if (strncmp(buffer, "square", count-1) == 0)
    {
        setMode(1, 4);
    }
    return count;
}

static ssize_t mode1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    mode1 = getMode(1);
	
    if(mode1 == 0)
	{
        strcpy(buffer, "DC\n");
	}
    else if(mode1 == 1)
	{
        strcpy(buffer, "Sine\n");
	}
    else if(mode1 == 2)
	{
        strcpy(buffer, "Sawtooth\n");
	}
    else if(mode1 == 3)
	{
        strcpy(buffer, "Triangle\n");
	}
    else if(mode1 == 4)
	{
        strcpy(buffer, "Square\n");
	}
    else if(mode1 == 5)
	{
        strcpy(buffer, "Arb\n");
	}
    return strlen(buffer);
}

static struct kobj_attribute mode1Attr = __ATTR(mode1, 0664, mode1Show, mode1Store);

static int freq1 = 0;
module_param(freq1, int, S_IRUGO);
MODULE_PARM_DESC(freq1, " Frequency of channel B");

static ssize_t freq1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &freq1);
    if (result == 0)
        setFreq(1, freq1);
    return count;
}

static ssize_t freq1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    freq1 = getFreq(1);
    return sprintf(buffer, "%d\n", freq1);
}

static struct kobj_attribute freq1Attr = __ATTR(freq1, 0664, freq1Show, freq1Store);

static int amplitude1 = 0;
module_param(amplitude1, int, S_IRUGO);
MODULE_PARM_DESC(amplitude1, " Amplitude of channel B");

static ssize_t amplitude1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &amplitude1);
    if (result == 0)
        setAmplitude(1, amplitude1);
    return count;
}

static ssize_t amplitude1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    amplitude1 = getAmplitude(1);
    return sprintf(buffer, "%d\n", amplitude1);
}

static struct kobj_attribute amplitude1Attr = __ATTR(amplitude1, 0664, amplitude1Show, amplitude1Store);


static int offset1 = 0;
module_param(offset1, int, S_IRUGO);
MODULE_PARM_DESC(offset1, " Offset of channel B");

static ssize_t offset1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtoint(buffer, 0, &offset1);
    if (result == 0)
        setOfs(1, offset1);
    return count;
}

static ssize_t offset1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    offset1 = getOfs(1);
    return sprintf(buffer, "%d\n", offset1);
}

static struct kobj_attribute offset1Attr = __ATTR(offset1, 0664, offset1Show, offset1Store);

static int dty1 = 0;
module_param(dty1, int, S_IRUGO);
MODULE_PARM_DESC(dty1, " Duty cycle of channel B");

static ssize_t dty1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &dty1);
    if (result == 0)
        setDty(1, dty1);
    return count;
}

static ssize_t dty1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    dty1 = getDty(1);
    return sprintf(buffer, "%d\n", dty1);
}

static struct kobj_attribute dty1Attr = __ATTR(dty1, 0664, dty1Show, dty1Store);

static int cycles1 = 0;
module_param(cycles1, int, S_IRUGO);
MODULE_PARM_DESC(cycles1, " Cycles of channel B");

static ssize_t cycles1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &cycles1);
	if(strncmp(buffer, "continuous", count-1) == 0)
    {
		setCycles(1, 0);
    }
    else if(result == 0)
        setCycles(1, cycles1);
    return count;
}

static ssize_t cycles1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    cycles1 = getCycles(1);
    return sprintf(buffer, "%d\n", cycles1);
}

static struct kobj_attribute cycles1Attr = __ATTR(cycles1, 0664, cycles1Show, cycles1Store);
// Enable 1

static unsigned int run = 0;
module_param(run, uint, S_IRUGO);
MODULE_PARM_DESC(run, " Run waves");

static ssize_t runStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "a", count-1) == 0)
    {
        runChannel(0);
		stopChannel(1);
		run = 1;
    }
    else if (strncmp(buffer, "b", count-1) == 0)
    {
        runChannel(1);
		stopChannel(0);
		run = 2;
    }
    else if (strncmp(buffer, "a+b", count-1) == 0)
    {
        runChannel(0);
		runChannel(1);
		run = 3;
    }
	else if (strncmp(buffer, "off", count-1) == 0)
    {
        stopChannel(0);
		stopChannel(1);
		run = 0;
    }
    return count;
}

static ssize_t runShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    run = isChannelRunning();
    if (run == 0)
	{
        strcpy(buffer, "off\n");
	}
    else if(run == 1)
	{
        strcpy(buffer, "a\n");
	}
	else if(run == 2)
	{
        strcpy(buffer, "b\n");
	}
	else if(run == 3)
	{
        strcpy(buffer, "a+b\n");
	}
    return strlen(buffer);
}

static struct kobj_attribute runAttr = __ATTR(run, 0664, runShow, runStore);

// Attributes

static struct attribute *attrs0[] = {&mode0Attr.attr, &freq0Attr.attr, &offset0Attr.attr, &amplitude0Attr.attr, &dty0Attr.attr, &cycles0Attr.attr,&runAttr.attr, NULL};
static struct attribute *attrs1[] = {&mode1Attr.attr, &freq1Attr.attr, &offset1Attr.attr, &amplitude1Attr.attr, &dty1Attr.attr, &cycles1Attr.attr,&runAttr.attr, NULL};

static struct attribute_group group0 =
{
    .name = "a",
    .attrs = attrs0
};

static struct attribute_group group1 =
{
    .name = "b",
    .attrs = attrs1
};

static struct kobject *kobj;

//-----------------------------------------------------------------------------
// Initialization and Exit
//-----------------------------------------------------------------------------

static int __init initialize_module(void)
{
    int result;

    printk(KERN_INFO "Wavegen driver: starting\n");

    // Create qe directory under /sys/kernel
    kobj = kobject_create_and_add("wavegen", NULL); //kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "Wavegen driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    // Create qe0 and qe1 groups
    result = sysfs_create_group(kobj, &group0);
    if (result !=0)
        return result;
    result = sysfs_create_group(kobj, &group1);
    if (result !=0)
        return result;

    // Physical to virtual memory map to access gpio registers
    base = (unsigned int*)ioremap(AXI4_LITE_BASE + WAVE_BASE_OFFSET,
                                          WAVE_SPAN_IN_BYTES);
    if (base == NULL)
        return -ENODEV;

    printk(KERN_INFO "Wavegen driver: initialized\n");

    return 0;
}

static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "Wavegen driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);

