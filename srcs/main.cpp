// @author Hieu Pham

#include <iostream>
#include <sstream>

#include <folly/Conv.h>
#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/exceptionString.h>
#include <folly/gen/Base.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

using namespace std;
using namespace folly;

DEFINE_string(input, "", "Input file name");
DEFINE_string(output, "tmp", "Output file name");
DEFINE_int64(move_by, -5, "Number of seconds to move by");

int64_t toUnix(int64_t hr, int64_t min, int64_t sec, int64_t sub) {
  return sub + 1000 * (sec + 60 * (min + 60 * (hr)));
}

std::string timeToString(int64_t time) {
  int64_t sub, sec, min, hr;
  sub = time % 1000;
  time /= 1000;

  sec = time % 60;
  time /= 60;

  min = time % 60;
  time /= 60;

  hr = time;
  return folly::format("{:02}:{:02}:{:02},{:03}", hr, min, sec, sub).str();
}

std::pair<int64_t, int64_t> getTime(std::string& line) {
  int64_t startHr, startMin, startSec, startSub;
  int64_t endHr, endMin, endSec, endSub;
  sscanf(line.c_str(), "%lld:%lld:%lld,%lld --> %lld:%lld:%lld,%lld", &startHr,
         &startMin, &startSec, &startSub, &endHr, &endMin, &endSec, &endSub);
  return std::make_pair<int64_t, int64_t>(
      toUnix(startHr, startMin, startSec, startSub),
      toUnix(endHr, endMin, endSec, endSub));
}

struct SubData {
  int64_t id{0};
  int64_t startTime{0};
  int64_t endTime{0};
  std::vector<std::string> content;

  bool empty() { return id == 0; }

  void moveBy(int64_t sec) {
    startTime += sec * 1000;
    endTime += sec * 1000;
  }
};

std::ostream& operator<<(std::ostream& o, SubData& data) {
  if (data.empty()) {
    return o;
  }

  // Output to stream.
  o << data.id << "\n"
    << timeToString(data.startTime) << " --> " << timeToString(data.endTime)
    << "\n";
  for (auto& content : data.content) {
    o << content << "\n";
  }

  return o;
}

class ContentIterator {
 public:
  ContentIterator(std::vector<std::string>& lines) : lines_(lines) {}

  SubData data() {
    SubData ret;
    try {
      if (lines_[lineNum_].empty()) {
        lineNum_++;
        return ret;
      }

      ret.id = folly::to<int64_t>(lines_[lineNum_++]);
      std::tie(ret.startTime, ret.endTime) = getTime(lines_[lineNum_++]);
      while (true) {
        auto& line = lines_[lineNum_++];
        ret.content.push_back(line);

        if (lineNum_ >= lines_.size() || line.empty()) {
          break;
        }
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << folly::exceptionStr(e);
    }

    return ret;
  }

  bool end() { return lineNum_ >= lines_.size(); }

 private:
  int64_t lineNum_{0};
  std::vector<std::string>& lines_;
};

void execute() {
  std::string content;
  std::vector<std::string> lines;
  std::stringstream stream;

  readFile(FLAGS_input.c_str(), content);
  folly::split("\n", content, lines);

  LOG(INFO) << folly::format("There are {} lines in subtitle", lines.size());

  ContentIterator it(lines);

  // Iterate through each subtitle data.
  while (!it.end()) {
    auto data = it.data();

    // Move by some seconds.
    data.moveBy(FLAGS_move_by);

    // Output to stream.
    stream << data;
  }

  writeFile(stream.str(), FLAGS_output.c_str());
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "INPUT: " << FLAGS_input;
  LOG(INFO) << "OUTPUT: " << FLAGS_output;

  // Execute
  execute();

  return 0;
}
