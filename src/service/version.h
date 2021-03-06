
#ifndef COMMON_VERSION_H_
#define COMMON_VERSION_H_

#define FASMIO_SDK_VERSION_MAJOR          0
#define FASMIO_SDK_VERSION_MINOR          1
#define FASMIO_SDK_VERSION                (FASMIO_SDK_VERSION_MAJOR * 100 + FASMIO_SDK_VERSION_MINOR)
#define FASMIO_SDK_VERSION_BUILDNUM       1000
#define FASMIO_SDK_VERSION_STR            #FASMIO_SDK_VERSION_MAJOR #FASMIO_SDK_VERSION_MINOR #FASMIO_SDK_VERSION_BUILDNUM

#define FASMIO_SDK_MAJOR(version)         ((version) / 100)
#define FASMIO_SDK_MINOR(version)         ((version) % 100)

#endif  // COMMON_VERSION_H_

