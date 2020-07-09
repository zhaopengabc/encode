/*
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * https://github.com/Bilibili/jni4android
 * This file is automatically generated by jni4android, do not modify.
 */

#include "IAndroidIO.h"

typedef struct J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO {
    jclass id;

    jmethodID method_open;
    jmethodID method_read;
    jmethodID method_seek;
    jmethodID method_close;
} J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO;
static J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO;

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open(JNIEnv *env, jobject thiz, jstring url)
{
    return (*env)->CallIntMethod(env, thiz, class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_open, url);
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open__catchAll(JNIEnv *env, jobject thiz, jstring url)
{
    jint ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open(env, thiz, url);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open__withCString(JNIEnv *env, jobject thiz, const char *url_cstr__)
{
    jint ret_value = 0;
    jstring url = NULL;

    url = (*env)->NewStringUTF(env, url_cstr__);
    if (J4A_ExceptionCheck__throwAny(env) || !url)
        goto fail;

    ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open(env, thiz, url);
    if (J4A_ExceptionCheck__throwAny(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    J4A_DeleteLocalRef__p(env, &url);
    return ret_value;
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open__withCString__catchAll(JNIEnv *env, jobject thiz, const char *url_cstr__)
{
    jint ret_value = 0;
    jstring url = NULL;

    url = (*env)->NewStringUTF(env, url_cstr__);
    if (J4A_ExceptionCheck__catchAll(env) || !url)
        goto fail;

    ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__open__catchAll(env, thiz, url);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    J4A_DeleteLocalRef__p(env, &url);
    return ret_value;
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__read(JNIEnv *env, jobject thiz, jbyteArray buffer, jint size)
{
    return (*env)->CallIntMethod(env, thiz, class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_read, buffer, size);
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__read__catchAll(JNIEnv *env, jobject thiz, jbyteArray buffer, jint size)
{
    jint ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__read(env, thiz, buffer, size);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jlong J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__seek(JNIEnv *env, jobject thiz, jlong offset, jint whence)
{
    return (*env)->CallLongMethod(env, thiz, class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_seek, offset, whence);
}

jlong J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__seek__catchAll(JNIEnv *env, jobject thiz, jlong offset, jint whence)
{
    jlong ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__seek(env, thiz, offset, whence);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__close(JNIEnv *env, jobject thiz)
{
    return (*env)->CallIntMethod(env, thiz, class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_close);
}

jint J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__close__catchAll(JNIEnv *env, jobject thiz)
{
    jint ret_value = J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO__close(env, thiz);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

int J4A_loadClass__J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO(JNIEnv *env)
{
    int         ret                   = -1;
    const char *J4A_UNUSED(name)      = NULL;
    const char *J4A_UNUSED(sign)      = NULL;
    jclass      J4A_UNUSED(class_id)  = NULL;
    int         J4A_UNUSED(api_level) = 0;

    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id != NULL)
        return 0;

    sign = "tv/danmaku/ijk/media/player/misc/IAndroidIO";
    class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id = J4A_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id == NULL)
        goto fail;

    class_id = class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id;
    name     = "open";
    sign     = "(Ljava/lang/String;)I";
    class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_open = J4A_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_open == NULL)
        goto fail;

    class_id = class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id;
    name     = "read";
    sign     = "([BI)I";
    class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_read = J4A_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_read == NULL)
        goto fail;

    class_id = class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id;
    name     = "seek";
    sign     = "(JI)J";
    class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_seek = J4A_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_seek == NULL)
        goto fail;

    class_id = class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.id;
    name     = "close";
    sign     = "()I";
    class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_close = J4A_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_J4AC_tv_danmaku_ijk_media_player_misc_IAndroidIO.method_close == NULL)
        goto fail;

    J4A_ALOGD("J4ALoader: OK: '%s' loaded\n", "tv.danmaku.ijk.media.player.misc.IAndroidIO");
    ret = 0;
fail:
    return ret;
}
