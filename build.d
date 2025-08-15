import std.datetime : SysTime;
import std.stdio : writeln, writefln;
import std.file : getTimes, rename, remove, copy, PreserveAttributes;
import core.stdc.stdlib : exit;
import std.process : wait, spawnProcess;
import std.algorithm : any, map;
import std.getopt;
import std.path : stripExtension;
import std.string : format;
import std.array : join;

string TARGET = "dmenu-desktop";
string[] SRC = ["main.d"];
string INSTALL_FOLDER = "/usr/local/bin";

const string RESET = "\033[0m";
const string GREEN = "\033[0;32m";
const string YELLOW = "\033[0;33m";
const string RED = "\033[0;31m";

SysTime getModificationTime(string fileName)
{
  SysTime accessTime;
  SysTime modificationTime;
  getTimes(fileName, accessTime, modificationTime);
  return modificationTime;
}

bool should_rebuild(string binary, string[] sources)
{
  try {
    SysTime binaryModTime = getModificationTime(binary);
    auto srcModTime = sources.map!(src => getModificationTime(src));
    return any!(a => a > binaryModTime)(srcModTime);
  } catch (Exception e) {
    return true;
  }
}

void logInfo(T)(string action, T description)
{
  writefln("[%s%s%s]: %s", GREEN, action, RESET, description);
}

void logWarning(T)(T description)
{
  writefln("[%sWARNING%s]: %s", YELLOW, RESET, description);
}

void logError(T)(T description)
{
  writefln("[%sERROR%s]: %s", RED, RESET, description);
}

int run(string[] args)
{
  logInfo("CMD", join(args, " "));
  auto status = wait(spawnProcess(args));

  if (status) {
    logError(format("exited %sabnormally%s with code %s%s%s", RED, RESET, RED, status, RESET));
  }

  return status;
}

int logAction(T)(string action, T description, void delegate() fn)
{
  logInfo(action, description);

  try {
    fn();
  } catch (Exception e) {
    logError(e.msg);
    return 1;
  }

  return 0;
}

int removeLog(string file)
{
  return logAction("REMOVE", file, () => remove(file));
}

int renameLog(string src, string target)
{
  return logAction("RENAME", src ~ " -> " ~ target, () => rename(src, target));
}

int copyLog(string src, string target)
{
  return logAction(
      "COPY", src ~ " -> " ~ target,
      () => copy(src, target, PreserveAttributes.yes
  ));
}

void rebuild_youself(string[] args)
{
    string binary = stripExtension(__FILE__);
    string file = __FILE__;

    if (!should_rebuild(binary, [file])) {
        return;
    }

    renameLog(binary, binary ~ ".old");
    auto status = run(["rdmd", "--build-only", file, "-of=" ~ binary]);

    if (status != 0) {
        writeln("[ERROR]: compilation failed");
        renameLog(binary ~ ".old", binary);
        exit(status);
    }

    removeLog(binary ~ ".old");

    run(["./" ~ binary] ~ args[1..$]);
    exit(0);
}

int build()
{
  if (!should_rebuild(TARGET, SRC)) {
    return 0;
  }

  return run(["rdmd", "--build-only", "-of=" ~ TARGET] ~ SRC);
}

int clean()
{
  return removeLog(TARGET);
}

int install()
{
  int status = build();

  if (status != 0) {
    return status;
  }

  return copyLog(TARGET, INSTALL_FOLDER ~ "/" ~ TARGET);
}

int uninstall()
{
  return removeLog(INSTALL_FOLDER ~ "/" ~ TARGET);
}

int main(string[] args)
{
  rebuild_youself(args);

  if (args.length == 1) {
    return build();
  }

  foreach (arg; args[1..$]) {
    switch (arg) {
      case "install": install(); break;
      case "clean": clean(); break;
      case "build": build(); break;
      case "uninstall": uninstall(); break;
      default: writefln("Unknown option '%s'", arg);
    }
  }

  return 0;
}
