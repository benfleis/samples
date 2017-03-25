package benfleis.samples.streams;

import java.time.ZonedDateTime;
import lombok.AllArgsConstructor;
import lombok.Value;

@Value
@AllArgsConstructor
class Payment {
    public int accountId;
    public int amountInCents;
    public ZonedDateTime timestamp;

    public Payment(int acctId, int amt, String iso8601Timestamp) {
        this(acctId, amt, ZonedDateTime.parse(iso8601Timestamp));
    }
}
