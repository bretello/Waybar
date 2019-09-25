#include "modules/disk.hpp"

using namespace waybar::util;

waybar::modules::Disk::Disk(const std::string& id, const Json::Value& config)
    : ALabel(config, "disk", id, "{}%", 30)
    , path_("/")
{
  thread_ = [this] {
    dp.emit();
    thread_.sleep_for(interval_);
  };
  if (config["path"].isString()) {
    path_ = config["path"].asString();
  }
}

auto waybar::modules::Disk::update() -> void {
  struct statvfs /* {
      unsigned long  f_bsize;    // filesystem block size
      unsigned long  f_frsize;   // fragment size
      fsblkcnt_t     f_blocks;   // size of fs in f_frsize units
      fsblkcnt_t     f_bfree;    // # free blocks
      fsblkcnt_t     f_bavail;   // # free blocks for unprivileged users
      fsfilcnt_t     f_files;    // # inodes
      fsfilcnt_t     f_ffree;    // # free inodes
      fsfilcnt_t     f_favail;   // # free inodes for unprivileged users
      unsigned long  f_fsid;     // filesystem ID
      unsigned long  f_flag;     // mount flags
      unsigned long  f_namemax;  // maximum filename length
  }; */ stats;
  int err = statvfs(path_.c_str(), &stats);

  /* Conky options
    fs_bar - Bar that shows how much space is used
    fs_free - Free space on a file system
    fs_free_perc - Free percentage of space
    fs_size - File system size
    fs_used - File system used space
  */

  if (err != 0) {
    event_box_.hide();
    return;
  }

  label_.set_markup(fmt::format(format_,
        stats.f_bavail * 100 / stats.f_blocks,
        fmt::arg("free", pow_format(stats.f_bavail * stats.f_bsize, "B", true)),
        fmt::arg("percentage_free", stats.f_bavail * 100 / stats.f_blocks),
        fmt::arg("used", pow_format((stats.f_blocks - stats.f_bavail) * stats.f_bsize, "B", true)),
        fmt::arg("percentage_used", (stats.f_blocks - stats.f_bavail) * 100 / stats.f_blocks)
  ));
  if (tooltipEnabled()) {
    label_.set_tooltip_text(fmt::format("{} used", pow_format(stats.f_bavail * stats.f_bsize, "B", true)));
  }
  event_box_.show();
}