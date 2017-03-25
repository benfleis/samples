package benfleis.samples.streams;

import java.util.Arrays;
import java.util.function.Function;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import lombok.val;

public final class Playground {
    public final static void main(String[] args) {
        // "stored" account information
        List<Account> accounts = Arrays.asList(
            new Account(1, "123-456", "alice"),
            new Account(2, "3324-01", "ben"),
            new Account(3, "11-3345", "charlie"),
            new Account(4, "5555", "ben")
        );

        Map<String, Account> accountsMap = accounts.stream()
            .collect(Collectors.toMap(Account::getAccount, Function.identity()));

        // payment batch
        List<Payment> payments = Arrays.asList(
            new Payment(1, 345, "2017-01-01T10:00:00+00:00"),
            new Payment(1, 100, "2017-01-01T10:15:00+00:00")
        );

        // join payments to accounts to generate paymentRequests

    }
}
