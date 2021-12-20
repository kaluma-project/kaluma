/* Copyright (c) 2017 Kaluma
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __FS_MAGIC_STRINGS_H
#define __FS_MAGIC_STRINGS_H

#define MSTR_FS_VFS_FLAG_READ "VFS_FLAG_READ"
#define MSTR_FS_VFS_FLAG_WRITE "VFS_FLAG_WRITE"
#define MSTR_FS_VFS_FLAG_CREATE "VFS_FLAG_CREATE"
#define MSTR_FS_VFS_FLAG_APPEND "VFS_FLAG_APPEND"
#define MSTR_FS_VFS_FLAG_EXCL "VFS_FLAG_EXCL"
#define MSTR_FS_VFS_FLAG_TRUNC "VFS_FLAG_TRUNC"

#define MSTR_FS_STATS "Stats"
#define MSTR_FS_STATS_TYPE "type"
#define MSTR_FS_STATS_SIZE "size"
#define MSTR_FS_STATS_IS_DIRECTORY "isDirectory"
#define MSTR_FS_STATS_IS_FILE "isFile"

#define MSTR_FS__PATH "__path"
#define MSTR_FS__FS "__fs"
#define MSTR_FS__CWD "__cwd"
#define MSTR_FS__LOOKUP "__lookup"
#define MSTR_FS__MOUNTS "__mounts"
#define MSTR_FS__OPENS "__opens"
#define MSTR_FS__PATHOUT "__pathout"
#define MSTR_FS_PATH "path"
#define MSTR_FS_BUFFER "buffer"
#define MSTR_FS_OFFSET "offset"
#define MSTR_FS_LENGTH "length"
#define MSTR_FS_POSITION "position"

#define MSTR_FS_FSTYPE_LFS "lfs"
#define MSTR_FS_FSTYPE_FAT "fat"

#define MSTR_FS_REGISTER "register"
#define MSTR_FS_UNREGISTER "unregister"
#define MSTR_FS_MOUNT "mount"
#define MSTR_FS_UNMOUNT "unmount"
#define MSTR_FS_CWD "cwd"
#define MSTR_FS_CHDIR "chdir"
#define MSTR_FS_CLOSE_SYNC "closeSync"
#define MSTR_FS_CREATE_READ_STREAM "createReadStream"
#define MSTR_FS_CREATE_WRITE_STREAM "createWriteStream"
#define MSTR_FS_EXISTS_SYNC "existsSync"
#define MSTR_FS_MKDIR_SYNC "mkdirSync"
#define MSTR_FS_RMDIR_SYNC "rmdirSync"
#define MSTR_FS_READDIR_SYNC "readdirSync"
#define MSTR_FS_OPEN_SYNC "openSync"
#define MSTR_FS_READ_SYNC "readSync"
#define MSTR_FS_READ_FILE_SYNC "readFileSync"
#define MSTR_FS_WRITE_SYNC "writeSync"
#define MSTR_FS_WRITE_FILE_SYNC "writeFileSync"
#define MSTR_FS_UNLINK_SYNC "unlinkSync"
#define MSTR_FS_RENAME_SYNC "renameSync"
#define MSTR_FS_STAT_SYNC "statSync"
#define MSTR_FS_RM_SYNC "rmSync"

#define MSTR_FS_BLKDEV "blkdev"
#define MSTR_FS_BLKDEV_READ "read"
#define MSTR_FS_BLKDEV_WRITE "write"
#define MSTR_FS_BLKDEV_IOCTL "ioctl"
#define MSTR_FS_FLASH = "Flash"

#endif /* __FS_MAGIC_STRINGS_H */
