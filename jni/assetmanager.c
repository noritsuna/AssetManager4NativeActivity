/*
   AssetManager for Android NDK
   Copyright (c) 2006-2013 SIProp Project http://www.siprop.org/

   This software is provided 'as-is', without any express or implied warranty.
   In no event will the authors be held liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it freely,
   subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <android/log.h>

#include "unzip.h"
#include "ioapi.h"

#define  LOG_TAG    "AssetManager"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#define DATA_PREFIX_PATH "/data/data"
#define PATH_LEN 512
#define PATH_LEN_PREFIX 19
#define APK_DIR_NUM 4
#define APK_LOAD_ERROR 100


int loadAseetFile_setAPKname(const char *package_name, const char *apk_name, const char *load_file_name) {

	int i;

	if(strlen(package_name) > PATH_LEN-PATH_LEN_PREFIX) {
		LOGE("Over Path Length: Max Path Length is PATH_LEN");
		return -1;
	}

	// Open APK File as Zip File
	char apk_path[APK_DIR_NUM][32] = {"/data/app",
									  "/data/app-private",
									  "/data/apk",
									  "/data/apk-private"};
	unzFile fp = NULL;
	for(i = 0; i < APK_DIR_NUM ; i++) {
		char apk_full_path[PATH_LEN] = {0};
		sprintf(apk_full_path, "%s/%s.apk", apk_path[i], apk_name);
		fp = unzOpen(apk_full_path);
		if(fp != NULL) break;
	}
	if(fp == NULL) {
		LOGE("Not Found APK File=[%s]", apk_name);
		return APK_LOAD_ERROR;
	}
	unz_file_info info;
	int hr = UNZ_OK;

	while(UNZ_OK == hr) {
		// Get Current File Information
		char data_path[PATH_LEN] = {0};
		char currentfile[PATH_LEN] = {0};
		hr = unzGetCurrentFileInfo(fp, &info, currentfile, 256, NULL, 0, NULL, 0);

		if(UNZ_OK == hr && strstr(currentfile, "assets") != NULL) {
			// Check "assets" dir is top dir or not.
			char manipulationstr[PATH_LEN] = {0};
			char *assets_dir;
			char *assets_next_dir;
			strcpy(manipulationstr, currentfile);
			assets_dir = strtok(manipulationstr, "/");
			if(assets_dir == NULL || strcmp(assets_dir, "assets") != 0 ) {
				// Go to Next File
				hr = unzGoToNextFile(fp);
				continue;
			}
			if(load_file_name != NULL) {
				if(strstr(currentfile, load_file_name) == NULL) {
					hr = unzGoToNextFile(fp);
					continue;
				}
			}

			// Make 2nd or more dirs.
			int rc = 0;
			sprintf(data_path, "%s/%s", DATA_PREFIX_PATH, package_name);
			do {
				// If next token is NULL, current token is File Name.
				assets_next_dir = strtok(NULL, "/");
				if(assets_next_dir == NULL) break;

				// Make new dir.
				sprintf(data_path, "%s/%s", data_path, assets_dir);
				rc = mkdir(data_path, S_IRWXU | S_IRWXG | S_IROTH);
				if (0 != rc && EEXIST != errno) {
					LOGE("Make Dir Got strerror(errno)=[%s]", strerror(errno));
				}

				assets_dir = assets_next_dir;
			} while(assets_dir != NULL);

			// Open File from APK
			hr = unzOpenCurrentFile(fp);
			char *buf = (char*)malloc(info.uncompressed_size + 1);
			memset(buf, 0, info.uncompressed_size + 1);
			unzReadCurrentFile(fp, buf, info.uncompressed_size);

			// Write File to assets dir
			sprintf(data_path, "%s/%s/%s", DATA_PREFIX_PATH, package_name, currentfile);
			FILE *new_fp = fopen(data_path, "w");
			if(new_fp == NULL) {
				LOGE("Make File Error. File Name=[%s]", data_path);
			} else {
				int fr = 0;
				fr = fwrite(buf, 1, info.uncompressed_size, new_fp);
				if(fr != info.uncompressed_size) {
					LOGE("Make File Error. File Name=[%s]", data_path);
				}
				fr = fclose(new_fp);
			}

			// Close File
			hr = unzCloseCurrentFile(fp);
			free(buf);
		}
		// Go to Next File
		hr = unzGoToNextFile(fp);
	}
	// Close APK File
	unzClose(fp);
}

int loadAseetFile(const char *package_name, const char *load_file_name) {
	int error_code = 0;
	int i;

	for( i = 1; i < 10; i++) {
		char apk_name[128] = {0};
		sprintf(apk_name, "%s-%d", package_name, i);
		error_code = loadAseetFile_setAPKname(package_name, apk_name, load_file_name);
		if(error_code != APK_LOAD_ERROR) break;
	}
	return error_code;
}


int setupAsset_setAPKname(const char *package_name, const char *apk_name) {
	return loadAseetFile_setAPKname(package_name, apk_name, NULL);
}

int setupAsset(const char *package_name) {
	int error_code = 0;
	int i;

	for( i = 1; i < 10; i++) {
		char apk_name[128] = {0};
		sprintf(apk_name, "%s-%d", package_name, i);
		error_code = setupAsset_setAPKname(package_name, apk_name);
		if(error_code != APK_LOAD_ERROR) break;
	}
	return error_code;
}

