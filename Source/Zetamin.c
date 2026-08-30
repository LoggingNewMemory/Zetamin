#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void setprop(const char* prop, const char* val) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "setprop %s \"%s\"", prop, val);
    system(cmd);
}

void setprop_int(const char* prop, long val) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%ld", val);
    setprop(prop, buf);
}

void resetprop_int(const char* prop, long val) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "resetprop %s %ld", prop, val);
    system(cmd);
}

void write_val(const char* path, const char* val) {
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s\n", val);
        fclose(f);
    }
}

void mask_val(const char* val, const char* path) {
    if (access(path, F_OK) != -1) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), 
            "umount \"%s\" 2>/dev/null; "
            "chmod 644 \"%s\" 2>/dev/null; "
            "echo \"%s\" > \"%s\"; "
            "touch /data/local/tmp/mount_mask; "
            "mount --bind /data/local/tmp/mount_mask \"%s\"", 
            path, path, val, path, path);
        system(cmd);
    }
}

void change_task_cgroup(const char* process_pattern, const char* cgroup_name, const char* cgroup_type) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
        "for temp_pid in $(ps -Ao pid,args | grep -i -E \"%s\" | grep -v PID | awk '{print $1}'); do "
        "for temp_tid in $(ls /proc/$temp_pid/task/ 2>/dev/null); do "
        "echo $temp_tid > /dev/%s/%s/tasks 2>/dev/null; "
        "done; done", 
        process_pattern, cgroup_type, cgroup_name);
    system(cmd);
}

long get_cmd_output_long(const char* cmd) {
    FILE *fp = popen(cmd, "r");
    long val = 0;
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            val = atol(buf);
        }
        pclose(fp);
    }
    return val;
}

void surfaceflinger_autoset(long ft, long thresh) {
    setprop_int("debug.sf.set_idle_timer_ms", thresh);
    setprop_int("debug.sf.phase_offset_threshold_for_next_vsync_ns", (ft / 6) + (thresh * 4800));
}

void other(long ft) {
    setprop("debug.sf.prime_shader_cache.solid_layers", "true");
    setprop("debug.sf.prime_shader_cache.image_layers", "true");
    setprop("debug.sf.prime_shader_cache.shadow_layers", "true");

    long cpu_time = get_cmd_output_long("awk -v b=$(cat /proc/sys/kernel/perf_cpu_time_max_percent 2>/dev/null||echo 25) '{n=$1/b;print int(35+(n*15)/(1+n))}' /proc/loadavg");
    setprop_int("debug.hwui.target_cpu_time_percent", cpu_time);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "awk -v ft=%ld 'BEGIN{printf \"%%.6f\", (ft/1000000000)*(ft<=10000000?0.85:0.75)}'", ft);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = 0;
            setprop("debug.sf.frame_rate_multiple_threshold", buf);
        }
        pclose(fp);
    }
}

void main_flux() {
    system("dumpsys SurfaceFlinger --latency-clear");
    sleep(1);
    
    long ft = get_cmd_output_long(
        "out=$(dumpsys SurfaceFlinger --latency 2>/dev/null | head -n 5); "
        "if echo \"$out\" | grep -Eq '^[0-9]+$|^[0-9]{10,}$'; then "
        "echo \"$out\" | head -n1 | grep -oE '[0-9]+'; "
        "else "
        "dumpsys SurfaceFlinger | grep -m1 -E \"VSYNC period|vsyncPeriod\" | awk '{print $7}' | grep -oE '[0-9]+'; "
        "fi"
    );

    long base_const, thresh;

    if (ft > 0) {
        base_const = (ft <= 13000000) ? 70 : 72;
        thresh = (ft / 1000000) + base_const + ((ft <= 13000000) ? 1 : 2);
    } else {
        ft = 16666667;
        base_const = 72;
        thresh = (ft / 1000000) + base_const + 2;
    }

    surfaceflinger_autoset(ft, thresh);
    system("set start vsync");
    other(ft);
}

void main_render() {
    // additional_gpu_settings
    write_val("/sys/module/ged/parameters/ged_smart_boost", "1000");
    write_val("/sys/module/ged/parameters/boost_upper_bound", "100");
    long fps = get_cmd_output_long("dumpsys display | grep -m1 \"mDefaultPeak\" | awk '{print int($2)}'");
    char fps_str[32]; snprintf(fps_str, sizeof(fps_str), "%ld", fps);
    write_val("/sys/module/ged/parameters/gx_dfps", fps_str);
    write_val("/sys/module/ged/parameters/g_gpu_timer_based_emu", "1");
    write_val("/sys/module/ged/parameters/boost_gpu_enable", "1");
    write_val("/sys/module/ged/parameters/ged_boost_enable", "1");
    write_val("/sys/module/ged/parameters/enable_gpu_boost", "1");
    write_val("/sys/module/ged/parameters/gx_game_mode", "1");
    write_val("/sys/module/ged/parameters/gx_boost_on", "1");
    write_val("/sys/module/ged/parameters/boost_amp", "1");
    write_val("/sys/module/ged/parameters/gx_3D_benchmark_on", "1");
    write_val("/sys/module/ged/parameters/is_GED_KPI_enabled", "1");
    write_val("/sys/module/ged/parameters/gpu_dvfs_enable", "1");
    write_val("/sys/module/ged/parameters/ged_monitor_3D_fence_disable", "0");
    write_val("/sys/module/ged/parameters/ged_monitor_3D_fence_debug", "0");
    write_val("/sys/module/ged/parameters/ged_log_perf_trace_enable", "0");
    write_val("/sys/module/ged/parameters/ged_log_trace_enable", "0");
    write_val("/sys/module/ged/parameters/gpu_bw_err_debug", "0");
    write_val("/sys/module/ged/parameters/gx_frc_mode", "0");
    write_val("/sys/module/ged/parameters/gpu_idle", "0");
    write_val("/sys/module/ged/parameters/gpu_debug_enable", "0");

    write_val("/sys/kernel/debug/ged/hal/gpu_boost_level", "2");
    write_val("/sys/kernel/debug/ged/hal/custom_upbound_gpu_freq", "1");
    
    write_val("/sys/devices/platform/gpu/dvfs_enable", "1");
    write_val("/sys/devices/platform/gpu/gpu_busy", "1");

    // optimize_gpu_frequency
    write_val("/proc/gpufreq/limit_table", "1 1 1");
    write_val("/proc/gpufreq/gpufreq_limited_thermal_ignore", "1");
    write_val("/proc/gpufreq/gpufreq_limited_oc_ignore", "1");
    write_val("/proc/gpufreq/gpufreq_limited_low_batt_volume_ignore", "1");
    write_val("/proc/gpufreq/gpufreq_limited_low_batt_volt_ignore", "1");
    write_val("/proc/gpufreq/gpufreq_fixed_freq_volt", "0");
    write_val("/proc/gpufreq/gpufreq_opp_stress_test", "0");
    write_val("/proc/gpufreq/gpufreq_power_dump", "0");
    write_val("/proc/gpufreq/gpufreq_power_limited", "0");
    write_val("/proc/gpufreqv2/aging_mode", "disable");

    // optimize_pvr_settings
    write_val("/sys/module/pvrsrvkm/parameters/gpu_power", "2");
    write_val("/sys/module/pvrsrvkm/parameters/HTBufferSizeInKB", "512");
    write_val("/sys/module/pvrsrvkm/parameters/DisableClockGating", "1");
    write_val("/sys/module/pvrsrvkm/parameters/EmuMaxFreq", "2");
    write_val("/sys/module/pvrsrvkm/parameters/EnableFWContextSwitch", "1");
    write_val("/sys/module/pvrsrvkm/parameters/gPVRDebugLevel", "0");
    write_val("/sys/module/pvrsrvkm/parameters/gpu_dvfs_enable", "1");
    
    write_val("/sys/kernel/debug/pvr/apphint/CacheOpConfig", "1");
    write_val("/sys/kernel/debug/pvr/apphint/CacheOpUMKMThresholdSize", "512");
    write_val("/sys/kernel/debug/pvr/apphint/EnableFTraceGPU", "0");
    write_val("/sys/kernel/debug/pvr/apphint/HTBOperationMode", "2");
    write_val("/sys/kernel/debug/pvr/apphint/TimeCorrClock", "1");
    write_val("/sys/kernel/debug/pvr/apphint/0/DisableFEDLogging", "1");
    write_val("/sys/kernel/debug/pvr/apphint/0/EnableAPM", "0");

    // optimize_adreno_driver
    long pwr_lvl = get_cmd_output_long("cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels 2>/dev/null");
    if (pwr_lvl > 0) pwr_lvl -= 1;
    char pwr_lvl_str[32]; snprintf(pwr_lvl_str, sizeof(pwr_lvl_str), "%ld", pwr_lvl);
    
    mask_val(pwr_lvl_str, "/sys/class/kgsl/kgsl-3d0/default_pwrlevel");
    mask_val(pwr_lvl_str, "/sys/class/kgsl/kgsl-3d0/min_pwrlevel");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/max_pwrlevel");
    mask_val("1", "/sys/class/kgsl/kgsl-3d0/bus_split");
    mask_val("1", "/sys/class/kgsl/kgsl-3d0/force_clk_on");
    mask_val("1", "/sys/class/kgsl/kgsl-3d0/force_no_nap");
    mask_val("1", "/sys/class/kgsl/kgsl-3d0/force_rail_on");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/force_bus_on");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/thermal_pwrlevel");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/perfcounter");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/throttling");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/fsync_enable");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/vsync_enable");
    mask_val("0", "/sys/class/kgsl/kgsl-3d0/devfreq/adrenoboost");

    write_val("/sys/kernel/debug/kgsl/kgsl-3d0/profiling/enable", "0");
    write_val("/sys/module/adreno_idler/parameters/adreno_idler_active", "0");
    write_val("/sys/module/msm_performance/parameters/touchboost", "1");

    // optimize_mali_driver
    write_val("/proc/mali/dvfs_enable", "1");
    system("mali_dir=$(ls -d /sys/devices/platform/soc/*mali*/scheduling 2>/dev/null | head -n 1); if [ -n \"$mali_dir\" ]; then echo \"full\" > \"$mali_dir/serialize_jobs\"; fi");
    system("mali1_dir=$(ls -d /sys/devices/platform/soc/*mali* 2>/dev/null | head -n 1); if [ -n \"$mali1_dir\" ]; then echo \"1\" > \"$mali1_dir/js_ctx_scheduling_mode\"; fi");

    // optimize_task_cgroup_nice
    change_task_cgroup("surfaceflinger", "", "cpuset");
    change_task_cgroup("system_server", "foreground", "cpuset");
    change_task_cgroup("netd|allocator", "foreground", "cpuset");
    change_task_cgroup("hardware.media.c2|vendor.mediatek.hardware", "background", "cpuset");
    change_task_cgroup("aal_sof|kfps|dsp_send_thread|vdec_ipi_recv|mtk_drm_disp_id|disp_feature|hif_thread|main_thread|rx_thread|ged_", "background", "cpuset");
    change_task_cgroup("pp_event|crtc_", "background", "cpuset");

    // final_optimize_gpu
    system("val=$(cat /sys/kernel/debug/fpsgo/common/gpu_block_boost 2>/dev/null); nf=$(echo \"$val\" | awk '{print NF}'); if [ \"$nf\" -eq 1 ]; then echo \"100\" > /sys/kernel/debug/fpsgo/common/gpu_block_boost; elif [ \"$nf\" -eq 3 ]; then echo \"60 120 1\" > /sys/kernel/debug/fpsgo/common/gpu_block_boost; fi");
    system("for p in $(find /sys/kernel/debug/tracing/events/pvr_fence -name 'enable' 2>/dev/null); do echo \"0\" > \"$p\"; done");

    write_val("/sys/kernel/debug/tracing/events/mtk_events/enable", "0");
    write_val("/proc/gpufreq/gpufreq_aging_enable", "0");

    write_val("/dev/cpuset/foreground/cpus", "0-3,4-7");
    write_val("/dev/cpuset/foreground/boost/cpus", "4-7");
    write_val("/dev/cpuset/top-app/cpus", "0-7");
}

void facur_main() {
    long max_fps = get_cmd_output_long("dumpsys display 2>/dev/null | grep -Eo 'fps=[0-9]+' | cut -d= -f2 | sort -nr | head -n1");
    if (max_fps == 0) max_fps = 60;
    
    long vsync_ns = 1000000000 / max_fps;
    long val_e = (vsync_ns * 80) / 100;
    long val_f = (vsync_ns * 60) / 100;
    long val_g = -val_e;
    long val_h = -val_f;

    const char* props_e[] = {
        "debug.sf.early.app.duration", "debug.sf.earlyGl.app.duration",
        "debug.sf.high_fps.early.app.duration", "debug.sf.high_fps.earlyGl.app.duration",
        "debug.sf.high_fps.late.app.duration", "debug.sf.late.app.duration"
    };
    for (int i = 0; i < 6; i++) setprop_int(props_e[i], val_e);

    const char* props_f[] = {
        "debug.sf.early.sf.duration", "debug.sf.earlyGl.sf.duration",
        "debug.sf.high_fps.early.sf.duration", "debug.sf.high_fps.earlyGl.sf.duration",
        "debug.sf.high_fps.late.sf.duration", "debug.sf.late.sf.duration"
    };
    for (int i = 0; i < 6; i++) setprop_int(props_f[i], val_f);

    const char* props_g[] = {
        "debug.sf.earlyGl_app_phase_offset_ns", "debug.sf.early_app_phase_offset_ns",
        "debug.sf.high_fps_earlyGl_app_phase_offset_ns", "debug.sf.high_fps_early_app_phase_offset_ns",
        "debug.sf.high_fps_late_app_phase_offset_ns", "debug.sf.late_app_phase_offset_ns"
    };
    for (int i = 0; i < 6; i++) setprop_int(props_g[i], val_g);

    const char* props_h[] = {
        "debug.sf.earlyGl_phase_offset_ns", "debug.sf.early_phase_offset_ns",
        "debug.sf.high_fps_earlyGl_phase_offset_ns", "debug.sf.high_fps_early_phase_offset_ns",
        "debug.sf.high_fps_late_phase_offset_ns", "debug.sf.late_phase_offset_ns"
    };
    for (int i = 0; i < 6; i++) setprop_int(props_h[i], val_h);
}

int main() {
    long max_rate = get_cmd_output_long("cmd display dump 2>/dev/null | grep -Eo 'fps=[0-9.]+' | cut -f2 -d= | sort -nr | head -n1 | cut -d . -f 1");
    if (max_rate > 60) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "settings put system min_refresh_rate %ld", max_rate); system(cmd);
        snprintf(cmd, sizeof(cmd), "settings put system peak_refresh_rate %ld", max_rate); system(cmd);
        resetprop_int("ro.surface_flinger.game_default_frame_rate_override", max_rate);
    }

    system("sync");
    main_flux();

    system("sync");
    main_render();

    system("sync");
    facur_main();

    return 0;
}
