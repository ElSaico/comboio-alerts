comboio-alerts
==============

## The Protocol

```
t e n <LF> u <LF> [m <LF>]
```
Spaces are merely for convenience of the reader and are not part of the data.

### Parameters

* ***t*** - target; can be one of:
    * **A** - alert bar
    * **L** - label
* ***e*** - event type; can be one of:
    * **F** - new follower
    * **S** - new subscription
    * **s** - subscription renewal
    * **G** - subscription gift
    * **C** - cheers
    * **R** - raid
* ***n*** - number related to event (months subscribed, amount of gifts/bits, users raiding, etc.; ignored for new follow)
* ***u*** - user who triggered the event
* ***m*** - additional message (alert bar only, not applicable for new follow and sub gift)
* **\<LF\>** - newline character
