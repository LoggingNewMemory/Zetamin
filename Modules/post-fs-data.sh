#!/system/bin/sh

# From EnCorinVest
resetprop debug.graphics.game_default_frame_rate.disabled true
resetprop sys.surfaceflinger.idle_reduce_framerate_enable no

# Vestia Zeta Display
resetprop debug.sf.disable_backpressure 1
resetprop debug.sf.disable_hwc 1
resetprop debug.sf.latch_unsignaled 1
resetprop debug.sf.disable_client_composition_cache 1
resetprop ro.surface_flinger.use_color_management false
resetprop ro.surface_flinger.has_wide_color_display false
resetprop ro.surface_flinger.has_hdr_display false
resetprop persist.sys.sf.native_mode 1
resetprop vendor.debug.mali.disable_afbc 1
resetprop ro.vendor.ddk.set.afbc 0
resetprop debug.gralloc.map_fb_memory 1
resetprop debug.gralloc.enable_fb_ubwc 0

# From BreezeOS
resetprop ro.max.fling_velocity 10000
resetprop ro.surface_flinger.max_frame_buffer_acquired_buffers 3
resetprop ro.surface_flinger.max_virtual_display_dimension 1920

# Zetamin
resetprop renderthread.skia.reduceopstasksplitting true
resetprop debug.hwui.skip_empty_damage true
resetprop debug.hwui.use_buffer_age true
resetprop ro.surface_flinger.use_context_priority true
resetprop debug.hwui.use_hint_manager true