#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sim_driver.h>
#include <android/log.h>

JNIEXPORT jint JNICALL Java_com_example_simplesim_DeviceController_openSim(JNIEnv *env, jobject obj) {
    int fd = open("/dev/sim_driver", O_RDWR);
    if (fd == -1) {
        perror("Failed to open device");
    }
    return fd;
}

JNIEXPORT jint JNICALL Java_com_example_simplesim_DeviceController_closeSim(JNIEnv *env, jobject obj, jint fd) {
	int ret = close(fd);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_example_simplesim_DeviceController_writeToDevice(JNIEnv *env, jobject obj, jint fd, jstring data) {
    const char *writeBuffer = (*env)->GetStringUTFChars(env, data, NULL);

    __android_log_print(ANDROID_LOG_DEBUG, "FPGA_device.c", "%s", writeBuffer);
    __android_log_print(ANDROID_LOG_DEBUG, "FPGA_device.c", "%d", strlen(writeBuffer));


    ssize_t bytesWritten = write(fd, "0000", strlen(writeBuffer));
    bytesWritten = write(fd, writeBuffer, strlen(writeBuffer));

    (*env)->ReleaseStringUTFChars(env, data, writeBuffer);

    if (bytesWritten == -1) {
        perror("Failed to write to device");
    }

    return bytesWritten;
}

JNIEXPORT jint JNICALL Java_com_example_simplesim_DeviceController_ioctlSetSim(JNIEnv *env, jobject obj, jint fd, jobjectArray data) {
    jsize length = (*env)->GetArrayLength(env, data);

    // Concatenate the strings from the Java array into a single message string
    char message[10] = "";
    jsize i;
    for (i = 0; i < length; i++) {
        jstring str = (jstring) (*env)->GetObjectArrayElement(env, data, i);
        const char *strChars = (*env)->GetStringUTFChars(env, str, NULL);
        if (i > 0) {
            strcat(message, " ");  // Add the separator
        }
        strcat(message, strChars);
        (*env)->ReleaseStringUTFChars(env, str, strChars);
    }

    //__android_log_print(ANDROID_LOG_DEBUG, "FPGA_device.c, ioctl set", "%s", message);

    // Perform the ioctl operation with the message string
    int result = ioctl(fd, SET_DATA, message);
    if (result == -1) {
        perror("Failed to perform ioctl");
    }

    return result;
}

JNIEXPORT jint JNICALL Java_com_example_simplesim_DeviceController_ioctlCmdSim(JNIEnv *env, jobject obj, jint fd, jstring data) {
    const char *message = (*env)->GetStringUTFChars(env, data, NULL);

    //__android_log_print(ANDROID_LOG_DEBUG, "FPGA_device.c, ioctl cmd", "%s", message);

    // Perform the ioctl operation with the message string
    int result = ioctl(fd, COMMAND, message);
    if (result == -1) {
        perror("Failed to perform ioctl");
    }
    (*env)->ReleaseStringUTFChars(env, data, message);

    return result;
}
