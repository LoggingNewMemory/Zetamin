#!/system/bin/sh
# ==============================================================================
# Zetamin: All in one Screen Tweaks
# Copyright (C) 2026 Kana Wesley Yamada
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
# ==============================================================================

# ------------------------------------------------------------------------------
# Revert Persistent Display & Refresh Rate Settings
# ------------------------------------------------------------------------------
revert_display_settings() {
    # System settings
    for key in \
        min_refresh_rate peak_refresh_rate user_refresh_rate \
        miui_refresh_rate oplus_customize_screen_refresh_rate \
        refresh_rate_mode unisoc.display_refreshrate all_refresh_rate \
        ScreenRefreshRateController_resultFps default_app_refresh_rate \
        other_apps_refresh_rate tran_other_app_refresh_rate \
        tran_need_recovery_refresh_rate tran_refresh_mode \
        tran_need_recovery_refresh_mode last_tran_refresh_mode_in_refresh_setting
    do
        settings delete system "$key" 2>/dev/null
    done

    # Global settings
    for key in \
        min_refresh_rate peak_refresh_rate \
        unisoc.display_refreshrate all_refresh_rate \
        ScreenRefreshRateController_resultFps
    do
        settings delete global "$key" 2>/dev/null
    done

    # Secure settings
    for key in \
        min_refresh_rate peak_refresh_rate miui_refresh_rate
    do
        settings delete secure "$key" 2>/dev/null
    done
}

# ------------------------------------------------------------------------------
# Revert Persistent System Properties
# ------------------------------------------------------------------------------
revert_persistent_props() {
    resetprop -p --delete persist.vendor.pnpmgr.gamemode.enable 2>/dev/null
    resetprop -d persist.vendor.pnpmgr.gamemode.enable 2>/dev/null
}

# ------------------------------------------------------------------------------
# Cleanup Leftover Temporary Files
# ------------------------------------------------------------------------------
cleanup_temp_files() {
    rm -f /data/local/tmp/zeta.prop 2>/dev/null
}

# Execute Cleanup Procedures
revert_display_settings
revert_persistent_props
cleanup_temp_files

# ------------------------------------------------------------------------------
# Standard Magisk File Cleanup
# ------------------------------------------------------------------------------
if [ -f "$INFO" ]; then
    while read -r LINE; do
        if [ "$(echo -n "$LINE" | tail -c 1)" = "~" ]; then
            continue
        elif [ -f "$LINE~" ]; then
            mv -f "$LINE~" "$LINE"
        else
            rm -f "$LINE"
            while true; do
                LINE=$(dirname "$LINE")
                [ "$(ls -A "$LINE" 2>/dev/null)" ] && break 1 || rm -rf "$LINE"
            done
        fi
    done < "$INFO"
    rm -f "$INFO"
fi

exit 0
