#include <stdint.h>
#include <pip/paging.h>
#include "minako.h"

extern char* _config;
extern char* _econfig;
extern task_t jsDescriptor;
extern const struct {uint32_t start, end;} js;
extern uint32_t curPart;
unsigned char* loaderPartition = 0;
uint32_t loaderPartitionSize = 0;

/* Executes a command line */
void Minako_ExecLine(char* line)
{
    /* Argument & command buffer */
    char cmd[256];
    char arg[256];

    /* Ignore comments */
    if(*line == '#')
        return;

    /* Find command */
    char* ptr = line;
    int index = 0;
    while(*ptr != ' ' && *ptr != '\0') {
        cmd[index] = *ptr;
        index++;
        ptr++;
    }; 

    cmd[index] = '\0';
    if(*ptr != '\0') /* Argument */
    {
        ptr++;
        index = 0;

        while(*ptr != '\0')
        {
            arg[index] = *ptr;
            index++;
            ptr++;
        }
        arg[index] = '\0';
    }
    /* Print command */
    if(strcmp(cmd, "print") == 0)
    {
        vgaputs(arg);
        vgaputs("\n");
    } 
    else if(strcmp(cmd, "file") == 0)
    {
        uint32_t size = rootfs_lookup(arg, &loaderPartition);
        if(size) {
            vgaputs("Selected ");
            vgaputs(arg);
            vgaputs(" as current partition image, ");
            vgaputdec(size);
            vgaputs(" bytes.\n");
            vgaputs("Copying to aligned location.\n");
            /* TODO : check the pages are contiguous; find proper location; add support for multiple partitions */
            uint32_t* pg = (uint32_t*)0x10A0000;
            memcpy(pg, loaderPartition, size);
            loaderPartition = (unsigned char*)pg;
            loaderPartitionSize = size;
        } else {
            vgaputs("Failed to set ");
            vgaputs(arg);
            vgaputs(" as current partition image.");
        }
    }
    /* Duktape loader */
    else if(strcmp(cmd, "duk_load") == 0)
    {
        uint32_t laddr  = strtol(arg, 0, 16);
        vgaputs("Loading Duktape into address ");
        vgaputhex(laddr);
        vgaputs("\n");
        if(BootstrapPartition((uint32_t)loaderPartition, loaderPartitionSize, laddr, &jsDescriptor)) {
            vgaputs("\tWarning: couldn't bootstrap partition.\n");
        } else {
            vgaputs("\tPartition loaded using Duktape loader.\n");
        }
    } 
    /* Boot command */
    else if(strcmp(cmd, "boot") == 0)
    {
        curPart = 0;
        Pip_VSTI();
    } 
    /* Unhandled */
    else {
        vgaputs("Unhandled command: ");
        vgaputs(cmd);
        vgaputs("\n");
    }

    return;
}

/* Parse a line - returns pointer to the next line */
char* Minako_ParseLine(char* line)
{
    char* ptr = line;
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    int index = 0;
    if(*line == '\n')
        return ptr+1;

    while(*ptr != '\n')
    {
        buffer[index] = *ptr;
        index++;
        ptr++;
    }

    /* NULL-terminate buffer */
    buffer[index] = '\0';

    /* Execute line */
    Minako_ExecLine(buffer);

    return ptr;
}

/* Parser for embedded configuration file */
void Minako_ExecConfig(unsigned char* file, int size)
{
    /* Display information */
    vgaputs("\nRunning embedded minako.cfg script.\n");

    char* cfg = (char*)file;
    char* ecfg = (char*)file + size;

    /* Parse until end of file */
    while (*cfg != '\0' && cfg < ecfg) {
        cfg = Minako_ParseLine(cfg);
    };
}
