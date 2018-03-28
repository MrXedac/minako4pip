#include <stdint.h>
#include <pip/api.h>
#include <pip/arch_api.h>
#include "minako.h"

uint16_t *textmemptr;
int attrib = 0x0F;
int csr_x = 0, csr_y = 0;

void scroll(void)
{
    uint16_t blank, temp;

    blank = 0x20 | (attrib << 8);

    if(csr_y >= 25)
    {
        temp = csr_y - 25 + 1;
        memcpy (textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2 * sizeof(uint16_t));

        memsetw (textmemptr + (25 - temp) * 80, blank, 80 * sizeof(uint16_t));
        csr_y = 25 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
 *  on the screen under the last character pressed! */
void move_csr(void)
{
    uint16_t temp;

    /* The equation for finding the index in a linear
     *  chunk of memory can be represented by:
     *  Index = [(y * width) + x] */
    temp = csr_y * 80 + csr_x;

    /* This sends a command to indicies 14 and 15 in the
     *  CRT Control Register of the VGA controller. These
     *  are the high and low bytes of the index that show
     *  where the hardware cursor is to be 'blinking'. To
     *  learn more, you should look up some VGA specific
     *  programming documents. A great start to graphics:
     *  http://www.brackeen.com/home/vga */
    Pip_Outb(0x3D4, 14);
    Pip_Outb(0x3D5, temp >> 8);
    Pip_Outb(0x3D4, 15);
    Pip_Outb(0x3D5, temp);
}

/* Clears the screen */
void cls()
{
    uint16_t blank;
    int i;

    /* Again, we need the 'short' that will be used to
     *  represent a space with color */
    blank = 0x20 | (attrib << 8);

    /* Sets the entire screen to spaces in our current
     *  color */
    for(i = 0; i < 25; i++)
        memsetw (textmemptr + i * 80, blank, 80 * sizeof(uint16_t));

    /* Update out virtual cursor, and then move the
     *  hardware cursor */
    csr_x = 0;
    csr_y = 0;
    move_csr();
}

/* vgaputs a single character on the screen */
void vgaputch(unsigned char c)
{
    uint16_t *where;
    uint16_t att = attrib << 8;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == 0x08)
    {
        if(csr_x != 0) csr_x--;
    }
    /* Handles a tab by incrementing the cursor's x, but only
     *  to a point that will make it divisible by 8 */
    else if(c == 0x09)
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
     *  cursor back to the margin */
    else if(c == '\r')
    {
        csr_x = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
     *  treat it as if a 'CR' was also there, so we bring the
     *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n')
    {
        csr_x = 0;
        csr_y++;
    }
    /* Any character greater than and including a space, is a
     *  printable character. The equation for finding the index
     *  in a linear chunk of memory can be represented by:
     *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = c | att;	/* Character AND attributes: color */
        csr_x++;
    }

    /* If the cursor has reached the edge of the screen's width, we
     *  insert a new line in there */
    if(csr_x >= 80)
    {
        csr_x = 0;
        csr_y++;
    }

    /* Scroll the screen if needed, and finally move the cursor */
    scroll();
    move_csr();
}

/* Uses the above routine to output a string... */
void vgaputs(char *text)
{
    int i;

    for (i = 0; text[i] != '\0'; i++)
    {
        vgaputch(text[i]);
    }
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(uint8_t forecolor, uint8_t backcolor)
{
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

#define H2C(n) ((n)<10?(n)+'0':(n)+'A'-10)
void vgaputhex(unsigned long n)
{
    int i;

    vgaputch('0'); vgaputch('x');
    for (i=0;i<8;i++){
        vgaputch(H2C((n>>28)&0xf));
        n<<=4;
    }
}

void vgaputdec(unsigned long n)
{
	char buf[16], *ptr = &buf[sizeof(buf)-1];

	if (!n){
		vgaputch('0');
		return;
	}

	for ( *ptr = 0; ptr > buf && n; n /= 10){
		*--ptr = '0'+(n%10);
	}

	vgaputs(ptr);
}
/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void)
{
    settextcolor(7, 0);
    textmemptr = (uint16_t *)0xC00B8000;
    cls();
}
