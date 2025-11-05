#!/usr/bin/env bash
# ============================================================
# Cross-platform system dependency installer for a C++ project
# Detects: apt / dnf|yum / pacman / zypper / brew / choco / winget
# Role: ONLY system deps (compiler, cmake, headers); NOT build project.
# Usage:
#   ./install_system_deps.sh               # normal, non-interactive
#   ./install_system_deps.sh --list        # show planned packages per manager
#   ./install_system_deps.sh --update-only # only update indexes
#   ./install_system_deps.sh --dry-run     # print actions, no changes
#   ./install_system_deps.sh --no-sudo     # run w/o sudo (CI root images)
# ============================================================

set -euo pipefail

# ---------- pretty logs ----------
info()    { echo -e "\033[1;34m[INFO]\033[0m $*"; }
warn()    { echo -e "\033[1;33m[WARN]\033[0m $*"; }
error()   { echo -e "\033[1;31m[ERROR]\033[0m $*"; }
success() { echo -e "\033[1;32m[SUCCESS]\033[0m $*"; }

# ---------- args ----------
DRY_RUN=0
UPDATE_ONLY=0
LIST_ONLY=0
USE_SUDO=1

for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=1 ;;
    --update-only) UPDATE_ONLY=1 ;;
    --list) LIST_ONLY=1 ;;
    --no-sudo) USE_SUDO=0 ;;
    -h|--help)
      cat <<EOF
Usage: $0 [--dry-run] [--update-only] [--list] [--no-sudo]
Installs system-level dependencies for this C++ project across popular package managers.
EOF
      exit 0
      ;;
    *) warn "Unknown arg: $arg" ;;
  esac
done

# ---------- sudo detection ----------
SUDO=""
if [[ $USE_SUDO -eq 1 ]]; then
  if [[ $EUID -ne 0 ]]; then
    if command -v sudo >/dev/null 2>&1; then
      SUDO="sudo"
    else
      warn "sudo not found; continuing without sudo."
      SUDO=""
    fi
  fi
fi

# ---------- manager detection ----------
PM=""
if   command -v apt-get >/dev/null 2>&1; then PM="apt"
elif command -v dnf      >/dev/null 2>&1; then PM="dnf"
elif command -v yum      >/dev/null 2>&1; then PM="yum"
elif command -v pacman   >/dev/null 2>&1; then PM="pacman"
elif command -v zypper   >/dev/null 2>&1; then PM="zypper"
elif command -v brew     >/dev/null 2>&1; then PM="brew"
elif command -v choco    >/dev/null 2>&1; then PM="choco"
elif command -v winget   >/dev/null 2>&1; then PM="winget"
else
  error "No supported package manager detected."
  exit 1
fi

# ---------- package maps per manager ----------
# 调整为你项目需要的集合（尽量选 CMake-aware 的 dev 包）
APT_PKGS=(
  build-essential cmake git pkg-config python3 python3-pip
  libeigen3-dev libfmt-dev libcgal-dev libboost-all-dev
)

DNF_PKGS=( # or YUM
  gcc gcc-c++ make cmake git pkgconf-pkg-config python3 python3-pip
  eigen3-devel fmt-devel CGAL boost-devel
)

PACMAN_PKGS=(
  base-devel cmake git pkgconf python python-pip
  eigen fmt cgal boost
)

ZYPER_PKGS=(
  gcc gcc-c++ make cmake git pkgconf-pkg-config python3 python3-pip
  eigen3-devel fmt-devel cgal-devel boost-devel
)

BREW_PKGS=( cmake git pkg-config python eigen fmt cgal boost )
# Windows：库管理分散，保留最常见工具；库请用 vcpkg/Conan 管理更靠谱
CHOCO_PKGS=( cmake git python ) # 可选: ninja
WINGET_IDS=(
  Kitware.CMake
  Git.Git
  Python.Python.3.12
)

# ---------- helpers ----------
run() { 
  if [[ $DRY_RUN -eq 1 ]]; then info "[dry-run] $*"; else eval "$@"; fi
}

update_indexes() {
  case "$PM" in
    apt)    run "$SUDO apt-get update -qq" ;;
    dnf)    run "$SUDO dnf makecache -y" ;;
    yum)    run "$SUDO yum makecache -y" ;;
    pacman) run "$SUDO pacman -Syu --noconfirm --needed" ;; # updates + refresh
    zypper) run "$SUDO zypper -n refresh" ;;
    brew)   info "Homebrew refresh not mandatory; skipping." ;;
    choco)  info "Chocolatey refresh not mandatory; skipping." ;;
    winget) info "Winget has no 'update index' concept; skipping." ;;
  esac
}

list_plan() {
  case "$PM" in
    apt)    printf '%s\n' "${APT_PKGS[@]}" ;;
    dnf|yum)printf '%s\n' "${DNF_PKGS[@]}" ;;
    pacman) printf '%s\n' "${PACMAN_PKGS[@]}" ;;
    zypper) printf '%s\n' "${ZYPER_PKGS[@]}" ;;
    brew)   printf '%s\n' "${BREW_PKGS[@]}" ;;
    choco)  printf '%s\n' "${CHOCO_PKGS[@]}" ;;
    winget) printf '%s\n' "${WINGET_IDS[@]}" ;;
  esac
}

install_all() {
  case "$PM" in
    apt)
      run "$SUDO apt-get install -y ${APT_PKGS[*]}"
      ;;
    dnf)
      run "$SUDO dnf install -y ${DNF_PKGS[*]}"
      ;;
    yum)
      run "$SUDO yum install -y ${DNF_PKGS[*]}"
      ;;
    pacman)
      # --needed 避免重复安装；pacman 前面已整体 -Syu 过
      run "$SUDO pacman -S --noconfirm --needed ${PACMAN_PKGS[*]}"
      ;;
    zypper)
      run "$SUDO zypper -n install ${ZYPER_PKGS[*]}"
      ;;
    brew)
      for p in "${BREW_PKGS[@]}"; do
        if brew list --versions "$p" >/dev/null 2>&1; then
          info "brew: $p already installed, skip."
        else
          run "brew install $p"
        fi
      done
      ;;
    choco)
      for p in "${CHOCO_PKGS[@]}"; do
        if choco list --local-only --exact "$p" | grep -q "^$p "; then
          info "choco: $p already installed, skip."
        else
          run "choco install -y --no-progress $p"
        fi
      done
      ;;
    winget)
      for id in "${WINGET_IDS[@]}"; do
        # 尝试精确 ID 安装；不同镜像下可能需要 -e -h / 同意协议
        run "winget install --id \"$id\" --accept-package-agreements --accept-source-agreements -e -h || true"
      done
      ;;
  esac
}

# ---------- main ----------
info "Detected package manager: $PM"
if [[ $LIST_ONLY -eq 1 ]]; then
  info "Planned packages/ids for $PM:"
  list_plan
  exit 0
fi

info "Updating indexes ..."
update_indexes
if [[ $UPDATE_ONLY -eq 1 ]]; then
  success "Indexes updated."
  exit 0
fi

info "Installing system dependencies ..."
install_all
success "System dependencies installed successfully."

