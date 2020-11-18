#!/bin/bash

#------------ Script Configurations: Start ------------#
SIN_BLOCK_SIZE=0x00080000
#------------ Script Configurations: END ------------#

# set -x

if [ "$1" == "sign_out_folder" ]; then
	SIN_ROOT_PATH=${REPO_ROOT}/out/target/product/${MYMTK_PROJECT};
	CERTDATA=${REPO_ROOT}/vendor/cci/tools/SIGNING/cert_data.dat
	FORMAL_SIGN="Y";
elif [ "$1" == "modem" ]; then
	SIN_ROOT_PATH=${REPO_ROOT}/out/target/product/${MYMTK_PROJECT};
	CERTDATA=${REPO_ROOT}/vendor/cci/tools/SIGNING/cert_data.dat
	FORMAL_SIGN="Y";
else
	SIN_ROOT_PATH=`pwd`;			
	CERTDATA=${SIN_ROOT_PATH}/cert_data.dat
fi


if [ "${FORMAL_SIGN}" == "Y" ]; then
	mkdir -p $SIN_ROOT_PATH/input
fi

if [ ! -e "$SIN_ROOT_PATH/output" ]; then
	echo "create $SIN_ROOT_PATH/output"
	mkdir -p $SIN_ROOT_PATH/output
else
	rm -r $SIN_ROOT_PATH/output
	mkdir -p $SIN_ROOT_PATH/output
fi

IMG2CHK="lk.bin logo.bin secro.img trustzone.bin boot.img system.img userdata.img cache.img recovery.img"
IMGCHK_ADDED=""
IMGCHK_EXIST=""

SINTOOL_ROOT=`pwd`

if [[ ${MYMTK_PROJECT} = "tuba" ]]; then
        CERT_NAME="PLATFORM-MT6755-TEST-88C8"
elif [[ ${MYMTK_PROJECT} = "smx1" ]] || [[ ${MYMTK_PROJECT} = "hinoki" ]] || [[ ${MYMTK_PROJECT} = "redwood" ]] || [[ ${MYMTK_PROJECT} = "teak" ]]; then
        CERT_NAME="PLATFORM-MT6757-TEST-2B8F"
else
        echo "Unsupported project ${MYMTK_PROJECT}"
        exit 0
fi
echo ${CERT_NAME}

function handle_check_image()
{
	for IMGCHK in ${IMG2CHK}; do
		if [ ! -f "$SIN_ROOT_PATH/$IMGCHK" ]; then
			echo "DUMMY" > $SIN_ROOT_PATH/$IMGCHK
			IMGCHK_ADDED="${IMGCHK_ADDED} $IMGCHK"
		else
			IMGCHK_EXIST="${IMGCHK_EXIST} $IMGCHK"
		fi
	done

	rm -rf $SIN_ROOT_PATH/signed_bin
	cd ${REPO_ROOT}
	./mk sign-image
	cd ${SINTOOL_ROOT}
	rm $SIN_ROOT_PATH/signed_bin/sro-lock-sign.img $SIN_ROOT_PATH/signed_bin/sro-unlock-sign.img
	rm $SIN_ROOT_PATH/sro-lock.img $SIN_ROOT_PATH/sro-unlock.img

	for files in $SIN_ROOT_PATH/signed_bin/* ;
	do
		filename=$(basename "$files")
		mv $SIN_ROOT_PATH/signed_bin/$filename `echo "$SIN_ROOT_PATH/signed_bin/$filename" | sed s/-sign//g`
	done

	for IMGCHK in ${IMGCHK_ADDED}; do
		rm $SIN_ROOT_PATH/$IMGCHK
		rm $SIN_ROOT_PATH/signed_bin/$IMGCHK
	done 

	rm -rf $SIN_ROOT_PATH/ORG_IMG

	mkdir -p $SIN_ROOT_PATH/ORG_IMG
	echo "${IMGCHK_EXIST}"

	mkdir $SIN_ROOT_PATH/signed_bin/
	cp $SIN_ROOT_PATH/lk.bin $SIN_ROOT_PATH/signed_bin/
	cp $SIN_ROOT_PATH/logo.bin $SIN_ROOT_PATH/signed_bin/
	cp $SIN_ROOT_PATH/secro.img $SIN_ROOT_PATH/signed_bin/
	cp $SIN_ROOT_PATH/trustzone.bin $SIN_ROOT_PATH/signed_bin/

	for IMGCHK in ${IMGCHK_EXIST}; do
		if [ "$IMGCHK" == "lk.bin" -o "$IMGCHK" == "secro.img" -o "$IMGCHK" == "logo.bin" -o "$IMGCHK" == "trustzone.bin" ]; then
			mv $SIN_ROOT_PATH/$IMGCHK $SIN_ROOT_PATH/ORG_IMG/$IMGCHK
			cp $SIN_ROOT_PATH/signed_bin/$IMGCHK $SIN_ROOT_PATH/$IMGCHK
		fi
	done
}

function handle_recover_image()
{
	for IMGCHK in ${IMGCHK_EXIST}; do
		if [ "$IMGCHK" == "lk.bin" -o "$IMGCHK" == "secro.img" -o "$IMGCHK" == "logo.bin" -o "$IMGCHK" == "trustzone.bin" ]; then
			mv $SIN_ROOT_PATH/ORG_IMG/$IMGCHK $SIN_ROOT_PATH/$IMGCHK
		fi
	done

	rm -rf $SIN_ROOT_PATH/ORG_IMG/ $SIN_ROOT_PATH/signed_bin_saved/
	mv $SIN_ROOT_PATH/signed_bin/ $SIN_ROOT_PATH/signed_bin_saved/
}

function replace_fotakernel_with_recovery_image()
{
	echo "replace fotakernel with recovery image for temp factory reset solution"
	#mv $SIN_ROOT_PATH/output/fotakernel_S1-SW-TEST-B316-0001-MMC.sin $SIN_ROOT_PATH/output/fotakernel_S1-SW-TEST-B316-0001-MMC.sin.bak
	mv $SIN_ROOT_PATH/output/recovery_S1-SW-TEST-B316-0001-MMC.sin $SIN_ROOT_PATH/output/fotakernel_S1-SW-TEST-B316-0001-MMC.sin
}

function handle_sign()
{
    IMG_NAME=$1
    CERT_PART_NAME=$2
    PARTITION_INFO=$3
    INTSIGN_ENABLE=$4
    IMG_SUBNAME=$5
    SINTYPE=$6
    RENAME=$7

    IMG_NOW=${IMG_NAME}.${IMG_SUBNAME}
    IMG_READYTOSIN=${IMG_NAME}_unsin.img
    if [ "${RENAME}" != "" ]; then
    	IMG_SIGNED=${RENAME}.sin
    else
	IMG_SIGNED=${IMG_NAME}.sin
    fi

	if [ "${FORMAL_SIGN}" == "Y" ]; then
		if [ -f "$SIN_ROOT_PATH/$IMG_NOW" ]; then
			cp $SIN_ROOT_PATH/$IMG_NOW $SIN_ROOT_PATH/input/$IMG_NOW
		fi 
	fi


	if [ -f "$SIN_ROOT_PATH/input/$IMG_NOW" ]; then


        echo "$IMG_NOW exist."

		if [ "$SIGNTYPE" == "2" ]; then
			cp $SIN_ROOT_PATH/input/$IMG_NOW $SIN_ROOT_PATH/input/$IMG_READYTOSIN
			./signatory -i $SIN_ROOT_PATH/input/$IMG_READYTOSIN -c $CERT_PART_NAME -t $SINTYPE --add-partition-info $PARTITION_INFO --sin-block-size $SIN_BLOCK_SIZE -o $SIN_ROOT_PATH/output/$IMG_SIGNED
			rm -rf $SIN_ROOT_PATH/input/$IMG_READYTOSIN		
		else
			if [ "$INTSIGN_ENABLE" == "TRUE" ]; then
				echo "start to internal signing"
				echo "./signatory -f ${CERTDATA} -c $CERT_PART_NAME -t $SINTYPE $SIN_ROOT_PATH/input/$IMG_NOW -o $SIN_ROOT_PATH/input/$IMG_READYTOSIN"
				./signatory -f ${CERTDATA} -c $CERT_PART_NAME -t $SINTYPE $SIN_ROOT_PATH/input/$IMG_NOW -o $SIN_ROOT_PATH/input/$IMG_READYTOSIN

			else
				if [ -f "$SIN_ROOT_PATH/input/$IMG_READYTOSIN" ]; then
					echo "$IMG_READYTOSIN exist., start to signing"
				else
					echo "$IMG_READYTOSIN not exist., start to copy"
					cp $SIN_ROOT_PATH/input/$IMG_NOW $SIN_ROOT_PATH/input/$IMG_READYTOSIN

					if [ -f "$SIN_ROOT_PATH/input/$IMG_READYTOSIN" ]; then
						echo "$IMG_READYTOSIN exist., start to signing"
					fi
				fi
				
				echo "signing image now..."
				./signatory -f ${CERTDATA} -c $CERT_PART_NAME --sin-block-size $SIN_BLOCK_SIZE --sin-gpt-guid $PARTITION_INFO -t $SINTYPE $SIN_ROOT_PATH/input/$IMG_READYTOSIN -o $SIN_ROOT_PATH/output/$IMG_SIGNED -n
				rm -r $SIN_ROOT_PATH/input/$IMG_READYTOSIN
			fi
		fi
    fi	
}

function modem_sign()
{
    IMG_NAME=$1
    CERT_PART_NAME=$2
    PARTITION_INFO=$3
    IMG_SUBNAME=$5
    SINTYPE=$6
    RENAME=$7

    IMG_NOW=${IMG_NAME}.${IMG_SUBNAME}
    if [ "${RENAME}" != "" ]; then
	IMG_SIGNED=${RENAME}.sin
    else
	IMG_SIGNED=${IMG_NAME}.sin
    fi

    ./signatory -f ${CERTDATA} -c $CERT_PART_NAME --sin-block-size $SIN_BLOCK_SIZE --sin-gpt-guid $PARTITION_INFO -t $SINTYPE $SIN_ROOT_PATH/signed_modem/$IMG_NOW -o $SIN_ROOT_PATH/output/$IMG_SIGNED -n
}

function modem_internal_sign()
{
    rm -rf ${SIN_ROOT_PATH}/signed_modem
    mkdir -p ${SIN_ROOT_PATH}/signed_modem
    ./signatory -f ${CERTDATA} -c ${CERT_NAME}-OEM0-MODEM -t MTKKBNMD -i ${SIN_ROOT_PATH}/md1img.img -o ${SIN_ROOT_PATH}/signed_modem/md1img_temp.img
    ./signatory -f ${CERTDATA} -c ${CERT_NAME}-OEM0-MODEM -t MTKKBN -i ${SIN_ROOT_PATH}/signed_modem/md1img_temp.img -o ${SIN_ROOT_PATH}/signed_modem/md1img.img
    ./signatory -f ${CERTDATA} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN -i ${SIN_ROOT_PATH}/md1dsp.img -o ${SIN_ROOT_PATH}/signed_modem/md1dsp.img
    ./signatory -f ${CERTDATA} -c ${CERT_NAME}-OEM0-EMBEDDED -t MTKKBN -i ${SIN_ROOT_PATH}/md1arm7.img -o ${SIN_ROOT_PATH}/signed_modem/md1arm7.img
    rm -rf ${SIN_ROOT_PATH}/signed_modem/md1img_temp.img
}

function help_function()
{
	echo "*******help_function*******"
	echo ""	
	echo "for version check"
	echo "	-v or -version"	
	echo ""	
	echo "for release note"	
	echo "	-r or -R"		
	echo ""		
	echo "*******help_function*******"
}

function version_function()
{
	echo "*******version_function*******"
	echo ""
	echo "signatory version info"
	./signatory -v
	echo ""
	echo "*******version_function*******"	
}

function releasenote_function()
{
	echo "*******releasenote_function*******"
	echo ""
	echo "=================================="
	echo "Inital version"
	echo "=================================="
	echo ""	
	echo "*******releasenote_function*******"
}

function signing_function()
{
	
	if [ "$1" == "sign_out_folder" ]; then
		echo "Checking images for signing BY57..."

		#handle_check_image

		#echo "start to internal signing"
		#cd ${REPO_ROOT}
		#./vendor/mediatek/proprietary/scripts/sign-image/sign_image.sh
		cd ${SINTOOL_ROOT}

		handle_sign	boot	S1-SW-TEST-B316-0001-ELF	2F50DBC8-FC14-93E4-4A9D-496FBA338B45	TRUE	img	ELF
		handle_sign	boot	S1-SW-TEST-B316-0001-MMC	2F50DBC8-FC14-93E4-4A9D-496FBA338B45	FALSE	img	SIN
		handle_sign	cache	S1-SW-TEST-B316-0001-MMC	A0844021-4CC1-3547-D2CD-0BDCC7B903C3	FALSE	img	SIN		

		if [ "$TARGET_BUILD_VARIANT" != "eng" ]; then
			handle_sign	system	DMVERITY-GOOGLE-REFERENCE-77E9-OEM0	F9CDF7BA-B834-A72A-F1C9-D6E0C0983896	TRUE	img	"DM_VERITY --dm-verity-use-sha256"
		fi

		handle_sign	system	S1-SW-TEST-B316-0001-MMC	F9CDF7BA-B834-A72A-F1C9-D6E0C0983896	FALSE	img	SIN	
		handle_sign	userdata	S1-SW-TEST-B316-0001-MMC	6EA4C6D3-957A-AB5B-7455-749893E94191	FALSE	img	SIN	
		
		#handle_sign	recovery	S1-SW-TEST-B316-0001-MMC	DBE0B276-4DB9-4A0D-ACA7-00684DE9C467	FALSE	img	SIN	
		
		#handle_sign	ltalabel	S1-ELABEL-LIVE-CF2A-PID1-0004-MMC	9DE21554-8B13-4E7D-B1E4-0371EB92073F	FALSE	img	SIN
		handle_sign	fotakernel	S1-SW-TEST-B316-0001-ELF	DBE0B276-4DB9-4a0d-ACA7-00684DE9C467	TRUE	img	ELF
		handle_sign	fotakernel	S1-SW-TEST-B316-0001-MMC	DBE0B276-4DB9-4A0D-ACA7-00684DE9C467	FALSE	img	SIN

		#handle_sign	md1img-verified	S1-SW-TEST-B316-0001-MMC	eb0aa2ea-42aa-445c-a7c4-d553c1b2b504	FALSE	img	SIN	md1img
		#handle_sign	md1dsp-verified	S1-SW-TEST-B316-0001-MMC	eeca5d5c-bf9f-4644-8f63-d59a9e091475	FALSE	img	SIN	md1dsp
		#handle_sign	md1arm7-verified	S1-SW-TEST-B316-0001-MMC	fc35cdc8-2c43-46cf-8166-9002afab992c	FALSE	img	SIN	md1arm7

		modem_internal_sign

		modem_sign	md1img	S1-SW-TEST-B316-0001-MMC	eb0aa2ea-42aa-445c-a7c4-d553c1b2b504	FALSE	img	SIN	md1img
		modem_sign	md1dsp	S1-SW-TEST-B316-0001-MMC	eeca5d5c-bf9f-4644-8f63-d59a9e091475	FALSE	img	SIN	md1dsp
		modem_sign	md1arm7	S1-SW-TEST-B316-0001-MMC	fc35cdc8-2c43-46cf-8166-9002afab992c	FALSE	img	SIN	md1arm7

		handle_sign	apps_log	S1-SW-TEST-B316-0001-MMC	467947FF-9B77-41A5-859E-BBC839B2B0BD	FALSE	img	SIN
		handle_sign	diag	S1-SW-TEST-B316-0001-MMC	D8DE1700-8075-11E4-B4A9-0800200C9A66	FALSE	img	SIN

		handle_sign	ramdump	S1-SW-TEST-B316-0001-ELF	E46574C6-0A59-4BCC-A6D0-D3827F1D67B9	TRUE	img	ELF
		handle_sign	ramdump	S1-SW-TEST-B316-0001-MMC	E46574C6-0A59-4BCC-A6D0-D3827F1D67B9	FALSE	img	SIN

		#handle_sign	oem	S1-SW-TEST-B316-0001-MMC	C83A6F3F-8B76-4B4D-9276-7B587C690E35	FALSE	img	SIN
		handle_sign	Qnovo	S1-SW-TEST-B316-0001-MMC	8421B5F7-378A-4BF5-A7C2-71E4DB82A0F5	FALSE	img	SIN

		handle_recover_image

		#replace_fotakernel_with_recovery_image

		echo "Signing BY57 End..."
		
		if [ "$FORMAL_SIGN" == "Y" ]; then
			rm -r $SIN_ROOT_PATH/input
		fi

	elif [ "$1" == "modem" ]; then
		echo "Checking images for signing BY57..."

		cd ${SINTOOL_ROOT}

		handle_sign	md1img	${CERT_NAME}-OEM0-MODEM	eb0aa2ea-42aa-445c-a7c4-d553c1b2b504	TRUE	img	MTKKBNMD	md1img
		handle_sign	md1img	${CERT_NAME}-OEM0-MODEM	eb0aa2ea-42aa-445c-a7c4-d553c1b2b504	TRUE	img	MTKKBN	md1img
		handle_sign	md1img	S1-SW-TEST-B316-0001-MMC		eb0aa2ea-42aa-445c-a7c4-d553c1b2b504	FALSE	img	SIN	md1img
		handle_sign	md1dsp	${CERT_NAME}-OEM0-EMBEDDED	eeca5d5c-bf9f-4644-8f63-d59a9e091475	TRUE	img	MTKKBN	md1dsp
		handle_sign	md1dsp	S1-SW-TEST-B316-0001-MMC		eeca5d5c-bf9f-4644-8f63-d59a9e091475	FALSE	img	SIN	md1dsp
		handle_sign	md1arm7	${CERT_NAME}-OEM0-EMBEDDED	fc35cdc8-2c43-46cf-8166-9002afab992c	TRUE	img	MTKKBN	md1arm7
		handle_sign	md1arm7	S1-SW-TEST-B316-0001-MMC		fc35cdc8-2c43-46cf-8166-9002afab992c	FALSE	img	SIN	md1arm7

		echo "Signing modem End..."
	else
		echo "sorry, unknown parameter, please check your parameter"
	fi
}

echo "help function is support, using parameter -h or /h for usage"

#------------ Following is for nonsparse files ------------#
if [ "$1" == "-h" -o "$1" == "/h" -o "$1" == "/help" -o "$1" == "-help" ]; then
	help_function
elif [ "$1" == "-version" -o "$1" == "-v" ]; then
	version_function
elif [ "$1" == "-r" -o "$1" == "-R" ]; then
	releasenote_function	
else
	cat /etc/issue
	signing_function $1
fi
