# SPDX-License-Identifier: GPL-3.0-or-later
"""Local safety checks shared by the graphical update workflow."""
from contextlib import contextmanager
import fcntl
from pathlib import Path


@contextmanager
def update_lock(cache):
    """Serialize foreground/background checks for this desktop user."""
    cache = Path(cache)
    cache.mkdir(mode=0o700, parents=True, exist_ok=True)
    with (cache / 'update.lock').open('a') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            yield False
            return
        try:
            yield True
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)


def installation_plan(package, runner):
    """Dry-run with the same removal/recommendation policy as installation."""
    result = runner('/usr/bin/apt-get', '-s', '--no-remove', '--no-install-recommends', 'install', str(package))
    if result.returncode != 0:
        raise ValueError('APT cannot prepare this update. No packages were changed.\n' + (result.stderr or result.stdout)[-1800:])
    if any(line.startswith(('Remv ', 'Purg ')) for line in result.stdout.splitlines()):
        raise ValueError('APT proposes removing packages; update refused.')
    changes = [line for line in result.stdout.splitlines() if line.startswith(('Inst ', 'Conf '))]
    if not any(line.startswith('Inst linwin ') or line.startswith('Inst linwin:amd64 ') for line in changes):
        raise ValueError('APT did not propose installing LinWin; update refused.')
    if len(changes) > 20:
        raise ValueError('APT proposes too many changes for a safe review; use the system package manager.')
    return '\n'.join(changes)


def confirm_plan(package, runner, approved):
    if installation_plan(package, runner) != approved:
        raise ValueError('APT installation plan changed after approval. Restart the update to review it again.')
