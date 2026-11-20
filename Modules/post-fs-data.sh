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
# 1. SurfaceFlinger Shader Cache & HWUI Composition Tweaks (Static)
# ------------------------------------------------------------------------------
resetprop debug.sf.prime_shader_cache.solid_layers true
resetprop debug.sf.prime_shader_cache.image_layers true
resetprop debug.sf.prime_shader_cache.shadow_layers true
resetprop debug.sf.disable_client_composition_cache 0
resetprop debug.renderengine.backend skiaglthreaded
resetprop debug.hwui.skia_atrace_enabled false

# ------------------------------------------------------------------------------
# 2. Frame Rate Enforcement & Game Refresh Rate Overrides (Static)
# ------------------------------------------------------------------------------
resetprop debug.graphics.game_default_frame_rate.disabled true
resetprop sys.surfaceflinger.idle_reduce_framerate_enable no
resetprop ro.surface_flinger.enable_frame_rate_override false
resetprop ro.surface_flinger.use_content_detection_for_refresh_rate false

# ------------------------------------------------------------------------------
# 3. Signature Identification
# ------------------------------------------------------------------------------
resetprop vestia.zeta.is Cat
