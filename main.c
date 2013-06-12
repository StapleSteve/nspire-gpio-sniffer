/*
    TI-NSPIRE GPIO sniffer
    Copyright (C) 2013  Daniel Tang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <os.h>

#define disable_irq() TCT_Local_Control_Interrupts(-1)
#define enable_irq() TCT_Local_Control_Interrupts(0)

#define printf(...) do { \
		disable_irq(); \
		printf(__VA_ARGS__); \
		enable_irq(); \
	} while (0)

struct gpio_state {
	unsigned char dir, in, out;
};

void read_state(struct gpio_state *state) {
	volatile unsigned char *ptr = (volatile unsigned char *)0x90000000;
	int i;

	disable_irq();

	for (i=0; i<4; i++) {
		state[i].dir	= *(ptr + 0x10);
		state[i].out	= *(ptr + 0x14);
		state[i].in	= *(ptr + 0x18);

		ptr += 0x40;
	}

	enable_irq();
}

void diff_text(const char *type, const char *one_text, const char *zero_text,
		unsigned char prev, unsigned char now, unsigned char gpio) {
	const char *now_text;
	const char *prev_text;

	if (!!prev == !!now)
		return;

	prev_text = prev ? one_text : zero_text;
	now_text = now ? one_text : zero_text;

	printf("GPIO %d: %s %s=>%s\n", gpio, type, prev_text, now_text);
}

void diff_state(struct gpio_state *prev, struct gpio_state *now) {
	int i, bit, gpio = 0;

	for (i=0; i<4; i++) {
		for (bit=0; bit<8; bit++) {
			diff_text("DIR", "INPUT", "OUTPUT",
				prev[i].dir & (1<<bit),
				now[i].dir & (1<<bit),
				gpio);

			diff_text("OUT", "HIGH", "LOW",
				prev[i].out & (1<<bit),
				now[i].out & (1<<bit),
				gpio);

			/* diff_text("IN", "HIGH", "LOW",
				prev[i].in & (1<<bit),
				now[i].in & (1<<bit),
				gpio); */

			gpio++;
		}
	}

}

int main() {
	unsigned char index = 0;
	struct gpio_state state[2][4];

	read_state(state[0]);

	while (!on_key_pressed()) {
		struct gpio_state *prev = state[index];
		struct gpio_state *now = state[!index];
		index = !index;

		read_state(now);
		diff_state(prev, now);
	}

	return 0;
}
