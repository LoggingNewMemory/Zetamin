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
# 1. SurfaceFlinger Low-Latency & Gaming Pipeline
# ------------------------------------------------------------------------------
resetprop debug.sf.latch_unsignaled 1
resetprop debug.sf.set_binder_thread_rt 1
resetprop debug.sf.multithreaded_present 1
resetprop debug.sf.use_frame_rate_priority 1
resetprop debug.sf.no_vsyncs_on_screen_off 1
resetprop debug.sf.use_phase_offsets_as_durations 1
resetprop ro.sf.disable_triple_buffer 0
resetprop ro.surface_flinger.use_context_priority true
resetprop ro.surface_flinger.max_frame_buffer_acquired_buffers 3
resetprop ro.surface_flinger.support_kernel_idle_timer false
resetprop ro.surface_flinger.force_hwc_copy_for_virtual_displays true
resetprop ro.surface_flinger.use_smart_90_for_video false
resetprop ro.surface_flinger.set_display_power_timer_ms 0
resetprop ro.surface_flinger.set_touch_timer_ms 180
resetprop ro.surface_flinger.set_idle_timer_ms 180
resetprop debug.sf.enable_adpf_cpu_hint true

# ------------------------------------------------------------------------------
# 2. RenderEngine & HWUI Rendering Acceleration
# ------------------------------------------------------------------------------
resetprop debug.egl.hw 1
resetprop debug.egl.swapinterval 1
resetprop debug.renderengine.backend skiaglthreaded
resetprop debug.renderengine.restore_blur_step false
resetprop debug.hwui.skia_atrace_enabled false
resetprop debug.hwui.initialize_gl_always true
resetprop debug.hwui.use_hint_manager true
resetprop debug.hwui.use_buffer_age true
resetprop debug.hwui.use_partial_updates true
resetprop debug.hwui.early_preload_gl_context true
resetprop debug.hwui.skip_eglmanager_telemetry true
resetprop debug.sf.prime_shader_cache.solid_layers true
resetprop debug.sf.prime_shader_cache.image_layers true
resetprop debug.sf.prime_shader_cache.shadow_layers true
resetprop debug.sf.disable_client_composition_cache 0

# ------------------------------------------------------------------------------
# 3. Frame Rate Enforcement & Game Refresh Rate Overrides
# ------------------------------------------------------------------------------
resetprop debug.graphics.game_default_frame_rate.disabled true
resetprop sys.surfaceflinger.idle_reduce_framerate_enable no
resetprop ro.surface_flinger.enable_frame_rate_override false
resetprop ro.surface_flinger.use_content_detection_for_refresh_rate false

# ------------------------------------------------------------------------------
# 4. Touch Responsiveness & Real-Time Scheduling
# ------------------------------------------------------------------------------
resetprop ro.max.fling_velocity 10000
resetprop sys.use_fifo_ui 1
resetprop persist.vendor.pnpmgr.gamemode.enable 1
resetprop sys.hwc.gpu_perf_mode 1
resetprop vendor.perf.framepacing.enable 1
resetprop vendor.perf.gestureflingboost.enable true
resetprop vendor.perf.workloadclassifier.enable true
resetprop vendor.boostfwk.scroll.policy.mode 1

# ------------------------------------------------------------------------------
# 5. Signature Identification
# ------------------------------------------------------------------------------
resetprop vestia.zeta.is Cat
