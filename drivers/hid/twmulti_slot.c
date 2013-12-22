/**
 * 	HID driver for the TimeLink MultiTouch panel,based on hid-multitouch.c,
 * 	the work of Stephane Chatty @ lii-enac;
 *
 * 	Copyright (c) 2011 ShenZhen Timelink Inc.
 *
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/input/mt.h>
/// #include "usbhid/usbhid.h"

MODULE_AUTHOR("fdeeg <fdeeg.li@gmail.com>");
MODULE_DESCRIPTION("TOUCHWIN multitouch driver");
MODULE_LICENSE("GPL");

/// #include "hid-ids.h"
#define USB_VENDOR_ID_TIMELINK 0x2309
#define USB_DEVICE_ID_TIMELINK 0x1001
#define twprint(fmt, ...) \
	printk(KERN_INFO "twmulti : "fmt"\n", ##__VA_ARGS__)

struct mt_slot {
	__s32 x, y, p, w, h;
	__s32 contactid;	/* the device ContactID assigned to this slot */
	bool touch_state;	/* is the touch valid? */
	bool seen_in_this_frame;/* has this slot been updated */
};

struct mt_device {
	struct mt_slot curdata;	/* placeholder of incoming data */
	struct mt_class *mtclass;	/* our mt device class */
	unsigned last_field_index;	/* last field index of the report */
	unsigned last_slot_field;	/* the last field of a slot */
	int last_mt_collection;	/* last known mt-related collection */
	__s8 inputmode;		/* InputMode HID feature, -1 if non-existent */
	__u8 num_received;	/* how many contacts we received */
	__u8 num_expected;	/* expected last contact index */
	__u8 maxcontacts;
	bool curvalid;		/* is the current contact valid? */
	struct mt_slot *slots;
};

struct mt_class {
	__s32 name;		/* MT_CLS */
	__s32 sn_move;		/* Signal/noise ratio for move events */
	__s32 sn_width;		/* Signal/noise ratio for width events */
	__s32 sn_height;	/* Signal/noise ratio for height events */
	__s32 sn_pressure;	/* Signal/noise ratio for pressure events */
	__u8 maxcontacts;
};

#define MT_DEFAULT_MAXCONTACT	5
#define MT_CLS_TOUCHWIN 	    0

static struct mt_class mt_classes[] = 
{
	/*
	 * vendor specific classes
	 */
	{ 	.name = MT_CLS_TOUCHWIN,
		.maxcontacts = 5,
		.sn_move     = 0,
		.sn_width    = 0,
		.sn_height   = 0 },
	{ }
};

static void mt_feature_mapping(struct hid_device *hdev,
		struct hid_field *field, struct hid_usage *usage)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	switch (usage->hid)
	{
	case HID_DG_INPUTMODE:
		td->inputmode = field->report->id;
		break;
	case HID_DG_CONTACTMAX:
		td->maxcontacts = field->value[0];
		if (td->mtclass->maxcontacts)
        {
            twprint("set the maxcontacts to %d", td->mtclass->maxcontacts);
			/* check if the maxcontacts is given by the class */
			td->maxcontacts = td->mtclass->maxcontacts;
        }
		break;
	}
}

static void set_abs(struct input_dev *input, unsigned int code,
		struct hid_field *field, int snratio)
{
	int fmin = field->logical_minimum;
	int fmax = field->logical_maximum;
	//int fuzz = snratio ? (fmax - fmin) / snratio : 0;
	input_set_abs_params(input, code, fmin, fmax, 0, 0);
}

static int mt_input_mapping(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	struct mt_class *cls = td->mtclass;

	twprint("mt_input_mapping");

	/* Only map fields from TouchScreen or TouchPad collections.
         * We need to ignore fields that belong to other collections
         * such as Mouse that might have the same GenericDesktop usages. */
	if (field->application == HID_DG_TOUCHSCREEN)
		set_bit(INPUT_PROP_DIRECT, hi->input->propbit);
	else if (field->application == HID_DG_TOUCHPAD)
		set_bit(INPUT_PROP_POINTER, hi->input->propbit);
	else
	  return 0;

	twprint("either from touchscreen or touchpad");
	switch (usage->hid & HID_USAGE_PAGE) 
	{
	case HID_UP_GENDESK:
		switch (usage->hid) 
		{
		case HID_GD_X:
			//field->logical_maximum = cls->sn_width;
			hid_map_usage(hi, usage, bit, max, EV_ABS, ABS_MT_POSITION_X);
			set_abs(hi->input, ABS_MT_POSITION_X, field, cls->sn_move);
			/* touchscreen emulation */
			set_abs(hi->input, ABS_X, field, cls->sn_move);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;

		case HID_GD_Y:
			//field->logical_maximum = cls->sn_height;
			hid_map_usage(hi, usage, bit, max, EV_ABS, ABS_MT_POSITION_Y);
			set_abs(hi->input, ABS_MT_POSITION_Y, field, cls->sn_move);
			/* touchscreen emulation */
			set_abs(hi->input, ABS_Y, field, cls->sn_move);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		}
		return 0;

	case HID_UP_DIGITIZER:
		switch (usage->hid)
		{
		case HID_DG_INRANGE:
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_CONFIDENCE:
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_TIPSWITCH:
			hid_map_usage(hi, usage, bit, max, EV_KEY, BTN_TOUCH);
			input_set_capability(hi->input, EV_KEY, BTN_TOUCH);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_CONTACTID:
			if (!td->maxcontacts)
				td->maxcontacts = MT_DEFAULT_MAXCONTACT;
			input_mt_init_slots(hi->input, td->maxcontacts);
			td->last_slot_field = usage->hid;
			td->last_field_index = field->index;
			td->last_mt_collection = usage->collection_index;
			return 1;
		case HID_DG_WIDTH:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_TOUCH_MAJOR);
			set_abs(hi->input, ABS_MT_TOUCH_MAJOR, field,
				cls->sn_width);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_HEIGHT:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_TOUCH_MINOR);
			set_abs(hi->input, ABS_MT_TOUCH_MINOR, field,
				cls->sn_height);
			input_set_abs_params(hi->input,
					ABS_MT_ORIENTATION, 0, 1, 0, 0);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_TIPPRESSURE:
			field->logical_minimum = 0;
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_PRESSURE);
			set_abs(hi->input, ABS_MT_PRESSURE, field,
				cls->sn_pressure);
			/* touchscreen emulation */
			set_abs(hi->input, ABS_PRESSURE, field,
				cls->sn_pressure);
			if (td->last_mt_collection == usage->collection_index) 
			{
				td->last_slot_field = usage->hid;
				td->last_field_index = field->index;
			}
			return 1;
		case HID_DG_CONTACTCOUNT:
			if (td->last_mt_collection == usage->collection_index)
				td->last_field_index = field->index;
			return 1;
		case HID_DG_CONTACTMAX:
			/* we don't set td->last_slot_field as contactcount and
			 * contact max are global to the report */
			if (td->last_mt_collection == usage->collection_index)
				td->last_field_index = field->index;
			return -1;
		}
		/* let hid-input decide for the others */
		return 0;

	case 0xff000000:
		/* we do not want to map these: no input-oriented meaning */
		return -1;
	}
	return 0;
}

static int mt_input_mapped(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	if (usage->type == EV_KEY || usage->type == EV_ABS)
		set_bit(usage->type, hi->input->evbit);
	return -1;
}

/*
 * this function is called when a whole contact has been processed,
 * so that it can assign it to a slot and store the data there
 */
static void mt_complete_slot(struct mt_device *td)
{
	td->curdata.seen_in_this_frame = true;
	if (td->curvalid) 
	{
		int slotnum = td->num_received;
		if (slotnum >= 0 && slotnum < td->maxcontacts)
			td->slots[slotnum] = td->curdata;
	}
	td->num_received++;
}

/*
 * this function is called when a whole packet has been received and processed,
 * so that it can decide what to send to the input layer.
 */
static void mt_emit_event(struct mt_device *td, struct input_dev *input)
{
	int i;
	//int s_x = 1280; // -100;
	//int s_y = 720; // - 100;

    /// twprint("input with number %d received and number expected %d", td->num_received, td->num_expected);
    /// twprint("maxcontacts %d", td->maxcontacts);

    for(i = 0; i < td->maxcontacts; i++)
	{
		struct mt_slot *s = &(td->slots[i]);
		if (!s->seen_in_this_frame || i >= td->num_expected) 
			s->touch_state = false;

		//s->x = ((int)s->x * s_x) / 32767;
		//s->y = ((int)s->y * s_y) / 32767; /// - 200
        //twprint("source x:%d y:%d", s->x, s->y);
        /// s->y = s->y * 2;
        /// twprint(" last y %d", s->y); 
        
        /// twprint("seen_in_this_frame %d", s->seen_in_this_frame);		
        /// twprint("input touch %d with x:%d y:%d and state %d", i, s->x, s->y, s->touch_state);

		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER,
			s->touch_state);
		if (s->touch_state) {
			input_event(input, EV_ABS, ABS_MT_POSITION_X, s->x);
			input_event(input, EV_ABS, ABS_MT_POSITION_Y, s->y);
			input_event(input, EV_ABS, ABS_MT_PRESSURE, s->p);
			input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, s->w);
			input_event(input, EV_ABS, ABS_MT_TOUCH_MINOR, s->h);
		}
		s->seen_in_this_frame = false;  
	}
	input_mt_report_pointer_emulation(input, true);
	input_sync(input);
	td->num_received = 0;
}

static int mt_event(struct hid_device *hid, struct hid_field *field,
				struct hid_usage *usage, __s32 value)
{
	struct mt_device *td = hid_get_drvdata(hid);
	/// twprint("mt_event");
	if (hid->claimed & HID_CLAIMED_INPUT && td->slots)
	 {
		switch (usage->hid) 
		{
		case HID_DG_INRANGE:
			/// twprint("HID_DG_INRANGE with %d", value);
			/// if (quirks & MT_QUIRK_VALID_IS_INRANGE)
			///	td->curvalid = value;
			break;
		case HID_DG_TIPSWITCH:
			/// twprint("HID_DG_TIPSWITCH with %d", value);
			td->curvalid 		= value;
			td->curdata.touch_state = value;
			break;
		case HID_DG_CONFIDENCE:
			/// twprint("HID_DG_CONFIDENCE with %d", value);
			/// if (quirks & MT_QUIRK_VALID_IS_CONFIDENCE)
			///	td->curvalid = value;
			break;
		case HID_DG_CONTACTID:
			/// twprint("HID_DG_CONTACTID with %d", value);
			td->curdata.contactid = value;
			break;
		case HID_DG_TIPPRESSURE:
			/// twprint("HID_DG_TIPPRESSURE with %d", value);
			td->curdata.p = value;
			break;
		case HID_GD_X:
			/// twprint("HID_GD_X with %d", value);
			td->curdata.x = value;
			break;
		case HID_GD_Y:
			/// twprint("HID_GD_Y with %d", value);
			td->curdata.y = value;
			break;
		case HID_DG_WIDTH:
			/// twprint("HID_DG_WIDTH with %d", value);
			td->curdata.w = value;
			break;
		case HID_DG_HEIGHT:
			/// twprint("HID_DG_HEIGHT with %d", value);
			td->curdata.h = value;
			break;
		case HID_DG_CONTACTCOUNT:
			/*
			 * Includes multi-packet support where subsequent
			 * packets are sent with zero contactcount.
			 */
			/// twprint("HID_DG_CONTACTCOUNT with %d", value);
			if (value)
				td->num_expected = value;
			break;
		default:
			/* fallback to the generic hidinput handling */
			return 0;
		}
		if (usage->hid == td->last_slot_field) 
		{
			/// twprint("last filed reached with %d-%d", td->curdata.x, td->curdata.y);
			mt_complete_slot(td);
		}

		if (field->index == td->last_field_index && td->num_received >= td->num_expected)
			mt_emit_event(td, field->hidinput->input);
	}
	/* we have handled the hidinput part, now remains hiddev */
	if (hid->claimed & HID_CLAIMED_HIDDEV && hid->hiddev_hid_event)
		hid->hiddev_hid_event(hid, field, usage, value);
	return 1;
}

static void mt_set_input_mode(struct hid_device *hdev)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	struct hid_report *r;
	struct hid_report_enum *re;

	if (td->inputmode < 0)
		return;

	re = &(hdev->report_enum[HID_FEATURE_REPORT]);
	r = re->report_id_hash[td->inputmode];
	if (r) 
	{
	   r->field[0]->value[0] = 0x02;
	   /// usbhid_submit_report(hdev, r, USB_DIR_OUT);
	}
}

static int mt_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret, i;
	struct mt_device *td;
	struct mt_class *mtclass = mt_classes; /* MT_CLS_DEFAULT */
	twprint("mt_probe");
	for (i = 0; mt_classes[i].name ; i++) 
	{
		if (id->driver_data == mt_classes[i].name)
		{
			mtclass = &(mt_classes[i]);
			break;
		}
	}
	twprint("probing with id_drvdata %d", (int)id->driver_data);

	td = kzalloc(sizeof(struct mt_device), GFP_KERNEL);
	if (!td) {
		dev_err(&hdev->dev, "cannot allocate multitouch data\n");
		return -ENOMEM;
	}
	td->mtclass = mtclass;
	td->inputmode = -1;
	td->last_mt_collection = -1;
	hid_set_drvdata(hdev, td);

	ret = hid_parse(hdev);
	if (ret != 0)
	   goto fail;

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret)
	   goto fail;

	td->slots = kzalloc(td->maxcontacts * sizeof(struct mt_slot), GFP_KERNEL);
	if (!td->slots) 
	{
		dev_err(&hdev->dev, "cannot allocate multitouch slots\n");
		hid_hw_stop(hdev);
		ret = -ENOMEM;
		goto fail;
	}
	mt_set_input_mode(hdev);
	return 0;
fail:
	kfree(td);
	return ret;
}

#ifdef CONFIG_PM
static int mt_reset_resume(struct hid_device *hdev)
{
	mt_set_input_mode(hdev);
	return 0;
}
#endif

static void mt_remove(struct hid_device *hdev)
{
	struct mt_device *td = hid_get_drvdata(hdev);
	hid_hw_stop(hdev);
	kfree(td->slots);
	kfree(td);
	hid_set_drvdata(hdev, NULL);
}

static const struct hid_device_id mt_devices[] = {
	/// TOUCHWIN device
	{ HID_USB_DEVICE ( USB_VENDOR_ID_TIMELINK , USB_DEVICE_ID_TIMELINK ) } ,
	{}
};
MODULE_DEVICE_TABLE(hid, mt_devices);

static const struct hid_usage_id mt_grabbed_usages[] = {
	{ HID_ANY_ID, HID_ANY_ID, HID_ANY_ID },
	{ HID_ANY_ID - 1, HID_ANY_ID - 1, HID_ANY_ID - 1}
};

static struct hid_driver mt_driver = {
	.name = "twmulti_slot",
	.id_table = mt_devices,
	.probe = mt_probe,
	.remove = mt_remove,
	.input_mapping = mt_input_mapping,
	.input_mapped = mt_input_mapped,
	.feature_mapping = mt_feature_mapping,
	.usage_table = mt_grabbed_usages,
	.event = mt_event,
#ifdef CONFIG_PM
	.reset_resume = mt_reset_resume,
#endif
};

static int __init mt_init(void)
{
    twprint("loaded");
	return hid_register_driver(&mt_driver);
}

static void __exit mt_exit(void)
{
    twprint("exit");
	hid_unregister_driver(&mt_driver);
}

module_init(mt_init);
module_exit(mt_exit);
