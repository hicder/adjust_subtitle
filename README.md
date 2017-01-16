ajust_subtitle
---------------------------

# Introduction

This program will help adjust the .srt subtitle file by certain amount of seconds.

# Prerequisites

This program aims to showcase some of Facebook and Google's opensource technology.

You will need:

- [buck](https://buckbuild.com/)
- [folly](https://github.com/facebook/folly)
- [gflags](https://github.com/gflags/gflags)
- [glog](https://github.com/google/glog)

You can install those with `homebrew`:

```
brew install folly
brew install gflags
brew install glog
```

# User manual

```
cd /path/to/adjust_subtitle
buck build srcs/...

./buck-out/gen/srcs/adjust_subtitle --input /path/to/old/sub.srt --output /path/to/new/sub.srt --move_by -5
```
