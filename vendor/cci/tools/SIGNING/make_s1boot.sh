#!/bin/bash

BUILD_NUMBER_VAR=`date "+%Y-%m-%d_%H:%M"`
PROJECT_NAME=${MYMTK_PROJECT}
MTK_BUILD_VER="O0.MP103"
BOOT_AID_VAR="1"
MTK_MASF_SIGNED="yes"

TOOL_ROOT=${REPO_ROOT}/vendor/cci/tools/SIGNING
BOOT_SIN_DIR=${REPO_ROOT}/out/target/product/${PROJECT_NAME}/s1boot_signbin
certdata=cert_data.dat

PL_IMG_NAME="preloader_${PROJECT_NAME}.bin.cat"

if [[ ${MTK_MASF_SIGNED} = "yes" ]]; then
#IMAGE_DIR=${REPO_ROOT}/out/target/product/${PROJECT_NAME}/signed_bin
#LK_IMG_NAME="lk-sign.bin"
#SECRO_IMG_NAME="secro-sign.img"
#LOGO_IMG_NAME="logo-sign.bin"
#TEE_IMG_NAME="trustzone-sign.bin"
IMAGE_DIR=${REPO_ROOT}/out/target/product/${PROJECT_NAME}
LK_IMG_NAME="lk-verified.img"
LOGO_IMG_NAME="logo-verified.bin"
TEE_IMG_NAME="tee-verified.img"
else
IMAGE_DIR=${REPO_ROOT}/out/target/product/${PROJECT_NAME}
LK_IMG_NAME="lk.bin"
LOGO_IMG_NAME="logo.bin"
TEE_IMG_NAME="tee.img"
fi

cd ${REPO_ROOT}

PLATFORM="mt67XX"
CERT_NAME="XXX"
if [[ ${PROJECT_NAME} = "tuba" ]]; then
	PLATFORM="mt6755"
	CERT_NAME="PLATFORM-MT6755-TEST-88C8"
elif [[ ${PROJECT_NAME} = "smx1" ]] || [[ ${PROJECT_NAME} = "hinoki" ]] || [[ ${PROJECT_NAME} = "redwood" ]] || [[ ${PROJECT_NAME} = "teak" ]]; then
	PLATFORM="mt6757"
	CERT_NAME="PLATFORM-MT6757-TEST-2B8F"
else
	echo "Unsupported platform ${PROJECT_NAME}"
	exit 0
fi

echo "PROJECT_NAME:$PROJECT_NAME"
echo "PLATFORM:${PLATFORM}"
echo "CERT_NAME:$CERT_NAME"

for boot_aid in ${BOOT_AID_VAR}; do
		make -j8 pl lk trustzone BOOT_AID=${boot_aid} BOOT_VER=${BUILD_NUMBER_VAR} BUILD_NUMBER=${MTK_BUILD_VER}
		if [ $? -ne 0 ]; then
			echo "Build error $?"
			break;
		else

			if [[ ${MTK_MASF_SIGNED} = "yes" ]]; then
				./vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh
			fi
			mkdir -p ${BOOT_SIN_DIR}
			rm ${BOOT_SIN_DIR}/*.*
			cd ${TOOL_ROOT}
			if [[ ${MTK_MASF_SIGNED} = "yes" ]]; then
echo "./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-ROOT -t MTKDAC ${IMAGE_DIR}/${PL_IMG_NAME} -o ${BOOT_SIN_DIR}/${PL_IMG_NAME}"
echo "./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/${LK_IMG_NAME}"
echo "./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${LOGO_IMG_NAME} -o ${BOOT_SIN_DIR}/${LOGO_IMG_NAME}"
echo "./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/${TEE_IMG_NAME}"


				./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-ROOT -t MTKDAC ${IMAGE_DIR}/${PL_IMG_NAME} -o ${BOOT_SIN_DIR}/${PL_IMG_NAME}
				./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/${LK_IMG_NAME}
				./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${LOGO_IMG_NAME} -o ${BOOT_SIN_DIR}/${LOGO_IMG_NAME}
				./signatory -f ${certdata} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/${TEE_IMG_NAME}
				cp ${BOOT_SIN_DIR}/${PL_IMG_NAME} ${IMAGE_DIR}/${PL_IMG_NAME}
				cp ${BOOT_SIN_DIR}/${LK_IMG_NAME} ${IMAGE_DIR}/${LK_IMG_NAME}
				cp ${BOOT_SIN_DIR}/${LOGO_IMG_NAME} ${IMAGE_DIR}/${LOGO_IMG_NAME}
				cp ${BOOT_SIN_DIR}/${TEE_IMG_NAME} ${IMAGE_DIR}/${TEE_IMG_NAME}
			fi
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --pldf-address-offset 0x800 --sin-block-size 0x00080000 --sin-gpt-guid 3bb4121a-469e-4a43-8533-63183929835e -t SIN ${IMAGE_DIR}/${PL_IMG_NAME} -o ${BOOT_SIN_DIR}/preloader_s1sbl.sin -n"
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 7EAF38FB-5939-9CB9-432A-A761CD06C9F7 -t SIN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/lk.sin -n"
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 33306389-E475-4AD1-B6AE-33E937E80D72 -t SIN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/lk2.sin -n"
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid C14BF896-7972-4525-BC81-489D0D15204F -t SIN ${IMAGE_DIR}/${LOGO_IMG_NAME} -o ${BOOT_SIN_DIR}/logo.sin -n"
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid E2404078-511C-402F-BB17-81A0AC08E61D -t SIN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/tee1.sin -n"
echo "./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 74455221-9b15-45ff-8fcf-4f011b6cf147 -t SIN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/tee2.sin -n"


			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --pldf-address-offset 0x800 --sin-block-size 0x00080000 --sin-gpt-guid 3bb4121a-469e-4a43-8533-63183929835e -t SIN ${IMAGE_DIR}/${PL_IMG_NAME} -o ${BOOT_SIN_DIR}/preloader_s1sbl.sin -n
			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 7EAF38FB-5939-9CB9-432A-A761CD06C9F7 -t SIN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/lk.sin -n
			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 33306389-E475-4AD1-B6AE-33E937E80D72 -t SIN ${IMAGE_DIR}/${LK_IMG_NAME} -o ${BOOT_SIN_DIR}/lk2.sin -n
			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid C14BF896-7972-4525-BC81-489D0D15204F -t SIN ${IMAGE_DIR}/${LOGO_IMG_NAME} -o ${BOOT_SIN_DIR}/logo.sin -n
			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid E2404078-511C-402F-BB17-81A0AC08E61D -t SIN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/tee1.sin -n
			./signatory -f ${certdata} -c S1-BOOT-TEST-B316-0001-MMC --sin-block-size 0x00080000 --sin-gpt-guid 74455221-9b15-45ff-8fcf-4f011b6cf147 -t SIN ${IMAGE_DIR}/${TEE_IMG_NAME} -o ${BOOT_SIN_DIR}/tee2.sin -n
			if [ -d ${BOOT_SIN_DIR} ]; then
				cd ${BOOT_SIN_DIR}
echo "zip -r s1boot_${PLATFORM}_V${BUILD_NUMBER_VAR}_AID${boot_aid}.zip *.sin"
				zip -r s1boot_${PLATFORM}_V${BUILD_NUMBER_VAR}_AID${boot_aid}.zip *.sin
				cp *.zip ${REPO_ROOT}/
				cd ${REPO_ROOT}
			fi 
		fi
done
