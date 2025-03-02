/*
 * Copyright (c) 2023 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "initializer.h"

#include <zephyr/dfu/mcuboot.h>


int main(void)
{

	if (!boot_is_img_confirmed()) {
		// Mark the ota image as installed so we don't revert
		printk("Confirming OTA update\n");

		int ret = boot_write_img_confirmed();
		if (ret == 0) {
			printk("Image confirmed successfully\n");
		} else {
			printk("Failed to confirm image: %d\n", ret);
		}		
    }
	//inizializza i threads
	initialize();

	return 0;
}
