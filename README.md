comboio-alerts
==============

## The Protocol

Spaces are merely for convenience of the reader and are not part of the messages.

* **M *t n...* \0 *u...* \0 [*m...* \0]** - display notification on the alert bar
* **L *t n...* \0 *u...* \0** - update label

### Parameters

* ***t*** - type of event; can be one of:
    * **F** - new follower
    * **S** - new subscription
    * **s** - subscription renewal
    * **G** - subscription gift
    * **C** - cheers
    * **R** - raid
* **\0** - null character
* ***n...*** - number related to event (months subscribed, amount of gifts/bits, users raiding, etc.; ignored for new follow)
* ***u...*** - user who triggered the event
* ***m...*** - additional message (subs and cheers only)