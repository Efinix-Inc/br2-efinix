################################################################################
#
# desktop_environment
#
################################################################################

DESKTOP_ENVIRONMENT_VERSION = 0.1
DESKTOP_ENVIRONMENT_DESCRIPTION = desktop environment configuration files
DESKTOP_ENVIRONMENT_SITE = $(BR2_EXTERNAL_EFINIX_PATH)/package/desktop_environment/src
DESKTOP_ENVIRONMENT_SITE_METHOD = local
DESKTOP_ENVIRONMENT_LICENSE = MIT
DESKTOP_ENVIRONMENT_DEPENDENCIES = xlib_libX11 openbox polybar picom hsetroot rofi font-awesome

define DESKTOP_ENVIRONMENT_SETUP_VC
	grep tty0 $(TARGET_DIR)/etc/inittab || \
	echo "tty0::respawn:/sbin/getty -L tty0 115200 vt100" >> $(TARGET_DIR)/etc/inittab
	chmod 644 $(TARGET_DIR)/etc/init.d/S40xorg
endef

define DESKTOP_ENVIRONMENT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/root
	$(INSTALL) -D -m 0755 $(@D)/S40startx $(TARGET_DIR)/etc/init.d/S40startx
	$(INSTALL) -D -m 0755 $(@D)/xinitrc $(TARGET_DIR)/root/.xinitrc
	$(INSTALL) -D -m 0644 $(@D)/xorg.conf $(TARGET_DIR)/etc/X11
	$(INSTALL) -D -m 0644 $(@D)/config/picom.conf $(TARGET_DIR)/root/.config/picom.conf
	$(INSTALL) -D -m 0644 $(@D)/config/polybar/config.ini $(TARGET_DIR)/root/.config/polybar/config.ini
	$(INSTALL) -D -m 0644 $(@D)/config/polybar/colors.ini $(TARGET_DIR)/root/.config/polybar/colors.ini
	$(INSTALL) -D -m 0644 $(@D)/config/polybar/modules.ini $(TARGET_DIR)/root/.config/polybar/modules.ini
	$(INSTALL) -D -m 0755 $(@D)/config/polybar/launch.sh $(TARGET_DIR)/root/.config/polybar/launch.sh
	$(INSTALL) -D -m 0755 $(@D)/config/openbox/autostart $(TARGET_DIR)/root/.config/openbox/autostart
	$(INSTALL) -D -m 0755 $(@D)/config/openbox/environment $(TARGET_DIR)/root/.config/openbox/environment
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/shared/colors.rasi $(TARGET_DIR)/root/.config/rofi/shared/colors.rasi
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/shared/fonts.rasi $(TARGET_DIR)/root/.config/rofi/shared/fonts.rasi
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/runner.rasi $(TARGET_DIR)/root/.config/rofi/runner.rasi
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/confirm.rasi $(TARGET_DIR)/root/.config/rofi/confirm.rasi
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/powermenu.rasi $(TARGET_DIR)/root/.config/rofi/powermenu.rasi
	$(INSTALL) -D -m 0644 $(@D)/config/rofi/launcher.rasi $(TARGET_DIR)/root/.config/rofi/launcher.rasi
	$(INSTALL) -D -m 0755 $(@D)/config/rofi/scripts/rofi-launcher $(TARGET_DIR)/root/.config/rofi/scripts/rofi-launcher
	$(INSTALL) -D -m 0755 $(@D)/config/rofi/scripts/rofi-runner $(TARGET_DIR)/root/.config/rofi/scripts/rofi-runner
	$(INSTALL) -D -m 0644 $(@D)/backgrounds/sungarden.png $(TARGET_DIR)/usr/share/backgrounds/sungarden.png
	$(INSTALL) -D -m 0644 $(@D)/backgrounds/mountains.png $(TARGET_DIR)/usr/share/backgrounds/mountains.png
	$(INSTALL) -D -m 0644 $(@D)/backgrounds/default.jpg $(TARGET_DIR)/usr/share/backgrounds/default.jpg
	$(INSTALL) -D -m 0755 $(@D)/config/openbox/autostart $(TARGET_DIR)/etc/xdg/openbox/autostart
	$(INSTALL) -D -m 0755 $(@D)/config/openbox/environment $(TARGET_DIR)/etc/xdg/openbox/environment
	$(INSTALL) -D -m 0644 $(@D)/config/windowlist/config.toml $(TARGET_DIR)/root/.config/windowlist/config.toml
	$(INSTALL) -D -m 0644 $(@D)/keycodes $(TARGET_DIR)/root/.keycodes
	$(INSTALL) -D -m 0644 $(@D)/mimeapps.list $(TARGET_DIR)/etc/xdg/mimeapps.list
endef

DESKTOP_ENVIRONMENT_POST_INSTALL_TARGET_HOOKS += DESKTOP_ENVIRONMENT_SETUP_VC

$(eval $(generic-package))
