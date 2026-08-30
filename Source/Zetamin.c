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

void write_val_dir(const char* dir, const char* file, const char* val) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, file);
    write_val(path, val);
}

void mask_val_dir(const char* val, const char* dir, const char* file) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, file);
    mask_val(val, path);
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

long get_real_fps() {
    long fps = get_cmd_output_long("cmd display dump 2>/dev/null | grep -Eo 'fps=[0-9.]+' | cut -f2 -d= | sort -nr | head -n1 | cut -d . -f 1");
    if (fps > 0) return fps;
    
    fps = get_cmd_output_long("dumpsys display 2>/dev/null | grep -Eo 'fps=[0-9.]+' | cut -d= -f2 | sort -nr | head -n1 | cut -d . -f 1");
    if (fps > 0) return fps;
    
    fps = get_cmd_output_long("dumpsys display 2>/dev/null | grep -i 'mDefaultPeak' | grep -Eo '[0-9]{2,3}' | head -n1");
    if (fps > 0) return fps;

    fps = get_cmd_output_long("dumpsys SurfaceFlinger 2>/dev/null | grep -i 'refresh-rate' | grep -Eo '[0-9]{2,3}' | head -n1");
    if (fps > 0) return fps;

    fps = get_cmd_output_long("dumpsys display 2>/dev/null | grep -i 'DisplayDeviceInfo' | grep -Eo 'fps [0-9.]+' | grep -Eo '[0-9]+' | sort -nr | head -n1");
    if (fps > 0) return fps;

    return 60; // Absolute fallback if all detections fail
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

void main_render(long max_rate) {
    long fps = max_rate;
    if (fps <= 0) fps = 60;
    char fps_str[32]; snprintf(fps_str, sizeof(fps_str), "%ld", fps);

    // additional_gpu_settings
    const char* ged = "/sys/module/ged/parameters";
    write_val_dir(ged, "gx_dfps", fps_str);
    write_val_dir(ged, "gx_game_mode", "1");
    write_val_dir(ged, "gx_3D_benchmark_on", "1");
    write_val_dir(ged, "is_GED_KPI_enabled", "1");
    write_val_dir(ged, "gpu_dvfs_enable", "1");
    write_val_dir(ged, "ged_monitor_3D_fence_disable", "0");
    write_val_dir(ged, "ged_monitor_3D_fence_debug", "0");
    write_val_dir(ged, "ged_log_perf_trace_enable", "0");
    write_val_dir(ged, "ged_log_trace_enable", "0");
    write_val_dir(ged, "gpu_bw_err_debug", "0");
    write_val_dir(ged, "gx_frc_mode", "0");
    write_val_dir(ged, "gpu_idle", "0");
    write_val_dir(ged, "gpu_debug_enable", "0");

    write_val("/sys/devices/platform/gpu/dvfs_enable", "1");

    // optimize_pvr_settings
    const char* pvr = "/sys/module/pvrsrvkm/parameters";
    write_val_dir(pvr, "HTBufferSizeInKB", "512");
    write_val_dir(pvr, "EnableFWContextSwitch", "1");
    write_val_dir(pvr, "gPVRDebugLevel", "0");
    write_val_dir(pvr, "gpu_dvfs_enable", "1");
    
    const char* pvr_app = "/sys/kernel/debug/pvr/apphint";
    write_val_dir(pvr_app, "CacheOpConfig", "1");
    write_val_dir(pvr_app, "CacheOpUMKMThresholdSize", "512");
    write_val_dir(pvr_app, "EnableFTraceGPU", "0");
    write_val_dir(pvr_app, "HTBOperationMode", "2");
    write_val_dir(pvr_app, "TimeCorrClock", "1");
    write_val_dir(pvr_app, "0/DisableFEDLogging", "1");

    // optimize_adreno_driver
    const char* kgsl = "/sys/class/kgsl/kgsl-3d0";
    mask_val_dir("1", kgsl, "bus_split");
    mask_val_dir("0", kgsl, "force_bus_on");
    mask_val_dir("0", kgsl, "perfcounter");
    mask_val_dir("0", kgsl, "fsync_enable");
    mask_val_dir("0", kgsl, "vsync_enable");
    mask_val_dir("0", kgsl, "devfreq/adrenoboost");

    write_val("/sys/kernel/debug/kgsl/kgsl-3d0/profiling/enable", "0");
    write_val("/sys/module/adreno_idler/parameters/adreno_idler_active", "0");

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
    system("for p in $(find /sys/kernel/debug/tracing/events/pvr_fence -name 'enable' 2>/dev/null); do echo \"0\" > \"$p\"; done");

    write_val("/sys/kernel/debug/tracing/events/mtk_events/enable", "0");

    const char* cpuset = "/dev/cpuset";
    write_val_dir(cpuset, "foreground/cpus", "0-3,4-7");
    write_val_dir(cpuset, "foreground/boost/cpus", "4-7");
    write_val_dir(cpuset, "top-app/cpus", "0-7");
}

void facur_main(long max_rate) {
    long max_fps = max_rate;
    if (max_fps <= 0) max_fps = 60;
    
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
    long max_rate = get_real_fps();
    if (max_rate > 60) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "settings put system min_refresh_rate %ld", max_rate); system(cmd);
        snprintf(cmd, sizeof(cmd), "settings put system peak_refresh_rate %ld", max_rate); system(cmd);
        resetprop_int("ro.surface_flinger.game_default_frame_rate_override", max_rate);
        system("resetprop ro.surface_flinger.enable_frame_rate_override false");
    }

    system("sync");
    main_flux();

    system("sync");
    main_render(max_rate);

    system("sync");
    facur_main(max_rate);

    return 0;
}
